using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace gc_backup_network
{
    public class FileRecvClient
    {

        private static List<FileRecvClient> fileRecvClients = new List<FileRecvClient>();

        private Socket clientSocket;
        private byte[] workBuffer = new byte[0x20000];

        private string? recvFilename = null;
        private UInt64? recvFilelength = null;
        private FileStream? receivingFile = null;
        private string readCString(byte[] data, int offset, int maxSize)
        {
            int slen = 0;
            for (int i = offset; i < offset + maxSize; i++)
                if (data[i] != 0) slen++;
                else break;

            return Encoding.UTF8.GetString(data, offset, slen);
        }

        private void parseHeader(byte[] header)
        {
            UInt16 magic = BitConverter.ToUInt16(header, 0);
            if (magic == 38717)
            {
                recvFilename = Path.GetFileName(readCString(header, 0x2, 0x200));
                recvFilelength = BitConverter.ToUInt64(header, 0x208);

                if (Path.Exists(recvFilename))
                    throw new Exception(recvFilename + " already exists.");

                receivingFile = File.OpenWrite(recvFilename);

                Console.WriteLine("Receiving file: " + recvFilename);
                Console.WriteLine("It is " + recvFilelength + " bytes long.");
            }
            else
            {
                throw new InvalidDataException("Invalid magic received");
            }
        }

        public FileRecvClient(Socket fromSocket) {
            this.clientSocket = fromSocket;

            // receive header from socket
            try
            {
                byte[] header = new byte[0x210];
                fromSocket.Receive(header, 0x00, header.Length, SocketFlags.None);
                parseHeader(header);
            }
            catch (Exception ex) { Console.Error.WriteLine(ex.Message); clientSocket.Disconnect(false); return; };

            SocketAsyncEventArgs e = new SocketAsyncEventArgs();
            e.Completed += receiveData;
            e.SetBuffer(workBuffer, 0, workBuffer.Length);

            fileRecvClients.Add(this);

            if (!clientSocket.ReceiveAsync(e))
                receiveData(null, e);
        }

        private void receiveData(object? sender, SocketAsyncEventArgs e)
        {
            do
            {
                if(e.Buffer is null) break;
                if (receivingFile is null) break;
                receivingFile.Write(e.Buffer, 0, e.BytesTransferred);
                if (Convert.ToUInt64(receivingFile.Length) >= recvFilelength)
                {
                    Console.WriteLine("File \""+recvFilename+"\" received.");
                    clientSocket.Disconnect(false);
                    receivingFile.Close();
                    return;
                }
            } while (!clientSocket.ReceiveAsync(e));
        }
    }
}
