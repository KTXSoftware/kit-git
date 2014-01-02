#ifndef NO_V8
#include <node.h>
#include <v8.h>
#endif
#include <git2.h>
#include <string.h>

#ifndef NO_V8
using namespace v8;

struct CloneData {
	char* from;
	char* to;
	Persistent<Function> callback;
};
#endif

struct Config {
	const char* serverDir;
	const char* projectsDir;
};

struct SubmoduleConfig {
	Config* config;
	const char* parentPath;
};

//int credentials(git_cred** cred, const char* url, const char* user_from_url, unsigned int allowed_types, void* payload) {
//	return git_cred_userpass_plaintext_new(cred, "turrican", "themachine");
//}

int initSubmodule(git_submodule* sm, const char* name, void* payload) {
	SubmoduleConfig* config = (SubmoduleConfig*)payload;

	git_submodule_init(sm, 0);
	
	git_repository* repo;
	git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
	
	char from[1001];
	strcpy(from, config->config->serverDir);
	strcat(from, &git_submodule_url(sm)[3]);

	char to[1001];
	strcpy(to, config->config->projectsDir);
	strcat(to, &git_submodule_url(sm)[3]);

	git_clone(&repo, from, to, &opts);

	char to2[1001];
	strcpy(to2, config->parentPath);
	to2[strlen(to2) - 5] = 0;
	strcat(to2, git_submodule_path(sm));

	git_clone(&repo, to, to2, &opts);

	SubmoduleConfig subConfig;
	subConfig.config = config->config;
	subConfig.parentPath = git_repository_path(repo);
	git_submodule_foreach(repo, initSubmodule, &subConfig);

	git_repository_free(repo);

	return 0;
}

void clone(char* project, Config* config) {
	git_repository* repo;
	git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
	//opts.cred_acquire_cb = credentials;
	//opts.transport_flags = GIT_TRANSPORTFLAGS_NO_CHECK_CERT;

	char from[1001];
	strcpy(from, config->serverDir);
	strcat(from, project);

	char to[1001];
	strcpy(to, config->projectsDir);
	strcat(to, project);

	git_clone(&repo, from, to, &opts);

	SubmoduleConfig subConfig;
	subConfig.config = config;
	subConfig.parentPath = git_repository_path(repo);
	git_submodule_foreach(repo, initSubmodule, &subConfig);

	git_repository_free(repo);
}

void update(char* dir) {
	git_repository* repo;
	git_repository_open(&repo, dir);

	git_remote* remote;
	git_remote_load(&remote, repo, "origin");
	git_remote_fetch(remote);
	git_remote_free(remote);

	git_reference* reference;
	git_branch_lookup(&reference, repo, "master", GIT_BRANCH_REMOTE);

	git_merge_head* merge_head;
	git_merge_head_from_ref(&merge_head, repo, reference);
	git_reference_free(reference);

	const git_merge_head* merge_heads[1];
	merge_heads[0] = merge_head;
	git_merge_result* result;
	git_merge(&result, repo, merge_heads, 1, NULL);
	git_repository_free(repo);
}

#ifndef NO_V8

void cloneWork(uv_work_t* req) {
	CloneData* cloneData = (CloneData*)req->data;
	clone(cloneData->from, cloneData->to);
}

void cloneAfter(uv_work_t* req) {
	HandleScope scope;
	CloneData* cloneData = (CloneData*)req->data;

	//Handle<Value> argv[] = {
	//	Null(),
	//	Number::New(asyncData->estimate)
	//};
	
	TryCatch try_catch;
	cloneData->callback->Call(Context::GetCurrent()->Global(), 0, nullptr);
	if (try_catch.HasCaught()) node::FatalException(try_catch);

	cloneData->callback.Dispose();

	delete[] cloneData->from;
	delete[] cloneData->to;
	delete cloneData;
	delete req;
}

Handle<Value> clone(const Arguments& args) {
	HandleScope scope;

	uv_work_t* req = new uv_work_t;
	CloneData* cloneData = new CloneData;
	req->data = cloneData;

	cloneData->from = new char[200];
	args[0]->ToString()->WriteAscii(cloneData->from, 0, 199);
	cloneData->to = new char[200];
	args[1]->ToString()->WriteAscii(cloneData->to, 0, 199);
	cloneData->callback = Persistent<Function>::New(Local<Function>::Cast(args[2]));

	uv_queue_work(uv_default_loop(), req, cloneWork, (uv_after_work_cb)cloneAfter);

	return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("clone"), FunctionTemplate::New(clone)->GetFunction());
}

NODE_MODULE(git, init)

#endif

int main(int argc, char** argv) {
	Config config;
	config.serverDir = "https://github.com/KTXSoftware/";
	config.projectsDir = "C:/Users/Robert/Projekte/Kit-Test/";
	clone("Kha", &config);
}
