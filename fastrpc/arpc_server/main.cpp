#include "rpc_server.h"
#include "echo.pb.h"

class EchoServiceImpl : public echo::EchoService {
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {

        response->set_response(request->message()+" server_hello");
        //RpcController* p_con = (RpcController*)controller;
        //unsigned cli_flow = p_con->_cli_flow;
        //CASyncSvr* svr = p_con->_svr;
        //RpcServer::PushToClient(svr, cli_flow, response); //这是服务器向客户端单向推消息
        if (done) {
            done->Run();
        }
    }
    virtual void Dummy(::google::protobuf::RpcController* controller,
                       const ::echo::DummyRequest* request,
                       ::echo::DummyResponse* response,
                       ::google::protobuf::Closure* done) {
        if (done) {
            done->Run();
        }
    }
public:
    RpcServer* _rpc_server;
public:
    EchoServiceImpl(RpcServer* rpc_server) {
        _rpc_server = rpc_server;
    }
};

class MyHttpHandler : public HttpHandler {
public:
    virtual void Init(CASyncSvr* svr) {}
    virtual void OnRec(HttpRequest* request,
                       ::google::protobuf::Closure *done) {
        CHttpParser* ps = request->ps;
        ps->parse_form_body();
        std::string kk = ps->get_param("kk");
        //string str_cmd = ps->get_object();
        //string get_uri = ps->get_uri();
        //std::stringstream ss;
        //ss << "kk:" << kk << "<br/>"
        //    << "cmd:" << str_cmd << "<br/>"
        //    << "uri:" << get_uri << "<br/>";

        std::string content_type = "text/html";
        std::string add_head = "Connection: keep-alive\r\n";
        CHttpResponseMaker::make_string(kk,
                                        request->response,
                                        content_type,
                                        add_head);

        if (done) {
            done->Run();
        }
    }
    virtual void Finish(CASyncSvr* svr) {}
};

int close_handler(CASyncSvr* svr, unsigned cli_flow, void* param) {
    //std::stringstream ss;
    //ss << "svr_id:" << svr->_svr_id //svr不是唯一的，可根据svr->_svr_id来区分
    //    << " cli:" << cli_flow //当前客户端socket的标示，不同svr有可能出现相同的cli_flow
    //    << " param:" << *((int*)param) << "\n";
    //所以svr_id结合cli_flow才是某一个客户端socket连接的唯一标示
    //printf(ss.str().c_str());
    return 0;
}

int main(int argc, char *argv[])
{
    RpcServer server("192.168.1.13", 8999);
    RPCREGI_ONEP(server, EchoServiceImpl, &server);
    HTTPREGI(server, MyHttpHandler);
    int p = 123;
    server.RegiClientCloseHandler(close_handler, &p); //对客户端掉线或关闭处理函数
    server.start();

    return 0;
}