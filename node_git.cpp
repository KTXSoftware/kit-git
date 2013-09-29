#include <node.h>
#include <v8.h>
#include <git2.h>

using namespace v8;

Handle<Value> Method(const Arguments& args) {
	git_repository* repo;
	git_clone(&repo, "https://github.com/KTXSoftware/Kore.git", "Kore", nullptr);
	HandleScope scope;
	return scope.Close(String::New("world"));
}

void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("hello"), FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(git, init)
