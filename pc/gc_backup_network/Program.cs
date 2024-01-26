using System.Net.Sockets;
using System.Net;
using System.Numerics;
using System.Text;

namespace gc_backup_network
{
    internal class Program
    {
        private static ManualResetEvent waitHandle = new ManualResetEvent(false);
        private static Socket? serverSocket = null;
        private static string getLocalIPAddress()
        {
            IPHostEntry host = Dns.GetHostEntry(Dns.GetHostName());
            string? ipAddr = null;
            foreach (IPAddress ip in host.AddressList)
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                    ipAddr = ip.ToString();

            if (ipAddr is not null) return ipAddr;
            else throw new Exception("No network adapters with an IPv4 address in the system!");
        }

        private static void startServer(string ip, UInt16 port)
        {
            serverSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            IPAddress hostIp = IPAddress.Parse(ip);
            IPEndPoint ep = new IPEndPoint(hostIp, port);
            serverSocket.Bind(ep);
            serverSocket.Listen(0x100);
            Console.WriteLine("Starting server on port : " + port);


            SocketAsyncEventArgs e = new SocketAsyncEventArgs();
            e.Completed += createNewClient;
            createNewClient(null, e);
        }


        private static void createNewClient(object? sender, SocketAsyncEventArgs e)
        {
            if (serverSocket is null) return;

            do
            {
                Socket? clientSocket = e.AcceptSocket;
                if (clientSocket is null) continue;
                new FileRecvClient(clientSocket);
                e.AcceptSocket = null;
            } while (!serverSocket.AcceptAsync(e));
        }

        static void Main(string[] args)
        {
            string ip = getLocalIPAddress();
            Console.WriteLine("Your local ip is : " + ip);
            UInt16 port = 46327;
            string listenIp = "0.0.0.0";

            if (args.Length >= 2)
            {
                listenIp = args[0];
                port = UInt16.Parse(args[1]);
            }
            startServer(listenIp, port);

            waitHandle.WaitOne();
        }
    }
}
