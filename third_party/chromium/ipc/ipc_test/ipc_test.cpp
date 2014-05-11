#include "third_party/chromium/base/threading/thread.h"
#include "third_party/chromium/ipc/ipc_descriptors.h"
#include "third_party/chromium/ipc/ipc_channel.h"
#include "third_party/chromium/ipc/ipc_channel_proxy.h"
#include "third_party/chromium/ipc/ipc_message_utils.h"
#include "third_party/chromium/ipc/ipc_switches.h"
#include "third_party/chromium/base/process_util.h"
#include "third_party/chromium/base/command_line.h"
#include "third_party/chromium/base/file_path.h"
#include "third_party/chromium/base/at_exit.h"
#include "third_party/chromium/base/time.h"

static void Send(IPC::Message::Sender* sender,
                 int index,
                 const char* text,
                 int benchmarkSize = 15)
{
  IPC::Message* message = new IPC::Message(0,
                                           2,
                                           IPC::Message::PRIORITY_NORMAL);
  message->WriteInt(index);
  message->WriteString(std::string(text));

  // Make sure we can handle large messages.
  message->WriteString(std::string(benchmarkSize, 'a'));

  // DEBUG: printf("[%u] sending message [%s]\n", GetCurrentProcessId(), text);
  sender->Send(message);
}

class ClientChannelListener : public IPC::Channel::Listener
{
public:
    virtual bool OnMessageReceived(const IPC::Message& message)
    {
        IPC::MessageIterator iter(message);

        int index = iter.NextInt();
        const std::string data = iter.NextString();
        const std::string big_string = iter.NextString();
        if (data != "bench")
            printf("client received: %s\n", data.c_str());
        
        if (data == "bye")
            MessageLoop::current()->Quit();
        else if (data == "world")
            BenchmarkNext(0);
        else if (data == "bench")
            BenchmarkNext(++index);
        else
            Send(sender_, 0, "what???");

        return true;
    }
    
    virtual void OnChannelError()
    {
        MessageLoop::current()->Quit();
    }
    
    void Init(IPC::Message::Sender* s)
    {
        sender_ = s;
    }
    
private:
    void BenchmarkNext(int index)
    {
        const int BENCH_TIMES = 10000;
        const int BENCH_SIZE = 1 * 1024 - 1;
        if (index == BENCH_TIMES)
        {
            base::Time now = base::Time::Now();
            base::TimeDelta t = now - sendTick_;
            printf("client benchmark %d times in %dms\n",
                   BENCH_TIMES, t.InMilliseconds());
            Send(sender_, 0, "bye", BENCH_SIZE);
        }
        else
        {
            if (index == 0)
                sendTick_ = base::Time::Now();
            Send(sender_, index, "bench", BENCH_SIZE);
        }
    }

    IPC::Message::Sender* sender_;
    base::Time sendTick_;
};

void client()
{
    MessageLoopForIO messageLoop;
    ClientChannelListener client_listener;
    IPC::Channel channel("ipc_test", IPC::Channel::MODE_CLIENT, &client_listener);
    int i = 10;
    for (; i > 0; --i)
    {
        if (channel.Connect())
        {
            printf("client connected\n");
            break;
        }
        Sleep(1000);
    }
    if (i == 0)
    {
        printf("client connect failed\n");
        return;
    }

    client_listener.Init(&channel);
    Send(&channel, 0, "hello");
    MessageLoop::current()->Run();
}

class ServerChannelListener : public IPC::Channel::Listener
{
public:
    virtual bool OnMessageReceived(const IPC::Message& message)
    {
        IPC::MessageIterator iter(message);

        int index = iter.NextInt();
        const std::string data = iter.NextString();
        const std::string big_string = iter.NextString();
        if (data != "bench")
            printf("server received: %s\n", data.c_str());

        if (data == "hello")
        {
            Send(sender_, 0, "world");
        }
        else if (data == "bench")
        {
            Send(sender_, index, "bench");
        }
        else if (data == "bye")
        {
            Send(sender_, 0, "bye");
            MessageLoop::current()->Quit();
        }
        else
        {
            Send(sender_, 0, "bye");
        }

        return true;
    }
    
    virtual void OnChannelError()
    {
        MessageLoop::current()->Quit();
    }
    
    void Init(IPC::Message::Sender* s)
    {
        sender_ = s;
    }
    
private:
    IPC::Message::Sender* sender_;
};

void server()
{
    MessageLoopForIO messageLoop;
    ServerChannelListener server_listener;
    IPC::Channel channel("ipc_test", IPC::Channel::MODE_SERVER, &server_listener);
    if (channel.Connect())
    {
        printf("connected\n");
    }
    else
    {
        printf("connect failed\n");
    }

    server_listener.Init(&channel);
    MessageLoop::current()->Run();
    getchar();
}

int wmain(int argc, wchar_t* argv[])
{
    base::AtExitManager at_exit_manager;

    if (argc == 1)
    {
        base::LaunchOptions options;
        CommandLine command((FilePath(argv[0])));
        command.AppendArg("client");
        base::LaunchProcess(command, options, NULL);
        server();
    }
    else
    {
        client();
    }
}
