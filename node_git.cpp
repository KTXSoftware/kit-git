#include <node.h>
#include <v8.h>
#include <git2.h>

using namespace v8;

struct CloneData {
	char* from;
	char* to;
	Persistent<Function> callback;
};

void cloneWork(uv_work_t* req) {
	CloneData* cloneData = (CloneData*)req->data;
	
	git_repository* repo;
	git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
	opts.transport_flags = GIT_TRANSPORTFLAGS_NO_CHECK_CERT;
	git_clone(&repo, cloneData->from, cloneData->to, &opts);
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
