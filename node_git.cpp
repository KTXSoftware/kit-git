#include <node.h>
#include <v8.h>
#include <git2.h>

using namespace v8;

struct CloneData {
	char* name;
	Persistent<Function> callback;
};

void cloneWork(uv_work_t* req) {
	CloneData* cloneData = (CloneData*)req->data;
	
	git_repository* repo;
	git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
	opts.transport_flags = GIT_TRANSPORTFLAGS_NO_CHECK_CERT;
	//git_clone(&repo, "https://git.ktxsoftware.com/git/Kha.git", "Kha", &opts);
	git_clone(&repo, "https://github.com/RobDangerous/Hurrican-Deployment.git", "Hurrican", &opts);
	git_repository_free(repo);
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

	delete cloneData;
	delete req;
}

Handle<Value> load(const Arguments& args) {
	HandleScope scope;

	uv_work_t* req = new uv_work_t;
	CloneData* cloneData = new CloneData;
	req->data = cloneData;

	//cloneData->name = 

	cloneData->callback = Persistent<Function>::New(Local<Function>::Cast(args[0]));

	uv_queue_work(uv_default_loop(), req, cloneWork, (uv_after_work_cb)cloneAfter);

	return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("load"), FunctionTemplate::New(load)->GetFunction());
}

NODE_MODULE(git, init)
