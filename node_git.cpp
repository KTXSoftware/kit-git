#ifndef NO_V8
#include <node.h>
#include <v8.h>
#endif
#include <git2.h>
#include <string.h>

struct Config {
	char* serverDir;
	char* projectsDir;
};

struct SubmoduleConfig {
	Config* config;
	const char* parentPath;
};

#ifndef NO_V8
using namespace v8;

struct UpdateData {
	char* project;
	Config config;
	Persistent<Function> callback;
};
#endif

//int credentials(git_cred** cred, const char* url, const char* user_from_url, unsigned int allowed_types, void* payload) {
//	return git_cred_userpass_plaintext_new(cred, "turrican", "themachine");
//}

void pull(git_repository* repo, const char* branch) {
	git_remote* remote;
	git_remote_load(&remote, repo, "origin");
	git_remote_fetch(remote);
	git_remote_free(remote);

	char remoteBranch[1001];
	strcpy(remoteBranch, "origin/");
	strcat(remoteBranch, branch);
	git_reference* reference;
	git_branch_lookup(&reference, repo, remoteBranch, GIT_BRANCH_REMOTE);

	git_merge_head* merge_head;
	git_merge_head_from_ref(&merge_head, repo, reference);
	git_reference_free(reference);

	const git_merge_head* merge_heads[1];
	merge_heads[0] = merge_head;
	git_merge_result* result;
	git_merge(&result, repo, merge_heads, 1, NULL);
}

int initSubmodule(git_submodule* sm, const char* name, void* payload) {
	SubmoduleConfig* config = (SubmoduleConfig*)payload;
	git_repository* repo;

	if (git_submodule_open(&repo, sm) < 0) {
		git_submodule_init(sm, 0);

		git_clone_options opts = GIT_CLONE_OPTIONS_INIT;

		char from[1001];
		strcpy(from, config->config->serverDir);
		strcat(from, &git_submodule_url(sm)[3]);

		char to[1001];
		strcpy(to, config->config->projectsDir);
		strcat(to, &git_submodule_url(sm)[3]);

		if (git_repository_open(&repo, to) == 0) pull(repo, git_submodule_branch(sm));
		else git_clone(&repo, from, to, &opts);
		git_repository_free(repo);

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
	}
	else {
		char global[1001];
		strcpy(global, config->config->projectsDir);
		strcat(global, &git_submodule_url(sm)[3]);

		git_repository* repo;
		
		git_repository_open(&repo, global);
		pull(repo, git_submodule_branch(sm));
		git_repository_free(repo);

		char local[1001];
		strcpy(local, config->parentPath);
		local[strlen(local) - 5] = 0;
		strcat(local, git_submodule_path(sm));

		git_repository_open(&repo, global);
		pull(repo, git_submodule_branch(sm));

		SubmoduleConfig subConfig;
		subConfig.config = config->config;
		subConfig.parentPath = git_repository_path(repo);
		git_submodule_foreach(repo, initSubmodule, &subConfig);

		git_repository_free(repo);
	}

	return 0;
}

void update(const char* project, Config* config) {
	char dir[1001];
	strcpy(dir, config->projectsDir);
	strcat(dir, project);

	git_repository* repo;
	if (git_repository_open(&repo, dir) == 0) {
		git_reference* ref;
		git_repository_head(&ref, repo);
		const char* branch;
		git_branch_name(&branch, ref);
		pull(repo, branch);

		SubmoduleConfig subConfig;
		subConfig.config = config;
		subConfig.parentPath = git_repository_path(repo);
		git_submodule_foreach(repo, initSubmodule, &subConfig);
	}
	else {
		git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
		//opts.cred_acquire_cb = credentials;
		//opts.transport_flags = GIT_TRANSPORTFLAGS_NO_CHECK_CERT;

		char from[1001];
		strcpy(from, config->serverDir);
		strcat(from, project);

		git_clone(&repo, from, dir, &opts);

		SubmoduleConfig subConfig;
		subConfig.config = config;
		subConfig.parentPath = git_repository_path(repo);
		git_submodule_foreach(repo, initSubmodule, &subConfig);
	}
	git_repository_free(repo);
}

#ifndef NO_V8

void updateWork(uv_work_t* req) {
	UpdateData* updateData = (UpdateData*)req->data;
	update(updateData->project, &updateData->config);
}

void updateAfter(uv_work_t* req) {
	HandleScope scope;
	UpdateData* updateData = (UpdateData*)req->data;

	//Handle<Value> argv[] = {
	//	Null(),
	//	Number::New(asyncData->estimate)
	//};
	
	TryCatch try_catch;
	updateData->callback->Call(Context::GetCurrent()->Global(), 0, nullptr);
	if (try_catch.HasCaught()) node::FatalException(try_catch);

	updateData->callback.Dispose();

	delete[] updateData->project;
	delete[] updateData->config.serverDir;
	delete[] updateData->config.projectsDir;
	delete updateData;
	delete req;
}

Handle<Value> update(const Arguments& args) {
	HandleScope scope;

	uv_work_t* req = new uv_work_t;
	UpdateData* updateData = new UpdateData;
	req->data = updateData;

	updateData->project = new char[1001];
	args[0]->ToString()->WriteAscii(updateData->project, 0, 1000);
	updateData->config.serverDir = new char[1001];
	args[1]->ToString()->WriteAscii(updateData->config.serverDir, 0, 1000);
	updateData->config.projectsDir = new char[1001];
	args[2]->ToString()->WriteAscii(updateData->config.projectsDir, 0, 1000);
	updateData->callback = Persistent<Function>::New(Local<Function>::Cast(args[3]));

	uv_queue_work(uv_default_loop(), req, updateWork, (uv_after_work_cb)updateAfter);

	return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("update"), FunctionTemplate::New(update)->GetFunction());
}

NODE_MODULE(git, init)

#else

int main(int argc, char** argv) {
	Config config;
	config.serverDir = "https://github.com/KTXSoftware/";
	config.projectsDir = "C:/Users/Robert/Projekte/Kit-Test/";
	update("Kha", &config);
}

#endif
