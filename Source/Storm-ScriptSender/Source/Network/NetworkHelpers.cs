using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Storm_ScriptSender.Source.Network
{
    public abstract class NetworkHelpers
    {
        private static int CopyInto(byte[] buff, byte[] toCopy, int offset)
        {
            int toCopySize = toCopy.Length;
            for (int iter = 0; iter < toCopySize; ++iter)
            {
                buff[iter + offset] = toCopy[iter];
            }
            return offset + toCopySize;
        }

        private static int CopyInto(byte[] buff, string asciiString, int offset)
        {
            int toCopySize = asciiString.Length;
            for (int iter = 0; iter < toCopySize; ++iter)
            {
                buff[iter + offset] = (byte)asciiString[iter];
            }
            return offset + toCopySize;
        }


        public static byte[] PrepareForSending(string scriptContent, Int32 currentPID, Storm.NetworkMessageType messageType)
        {
            int k_networkSepLength = Storm.NetworkConstants.k_networkSeparator.Length;
            int k_additionalSize =
                4 + // magicKeyword
                1 + // application type
                4 + // PID
                1 + // Message Type
                Storm.NetworkConstants.k_endOfMessageCommand.Length +
                k_networkSepLength * 5
                ;

            byte[] bytes = new byte[scriptContent.Length + k_additionalSize];

            int offset = CopyInto(bytes, BitConverter.GetBytes(IPAddress.HostToNetworkOrder(Storm.NetworkConstants.k_magicKeyword)), 0);

            offset = CopyInto(bytes, Storm.NetworkConstants.k_networkSeparator, offset);
            bytes[offset] = (byte)Storm.NetworkApplication.Storm_ScriptSender;
            offset += 1;

            offset = CopyInto(bytes, Storm.NetworkConstants.k_networkSeparator, offset);
            offset = CopyInto(bytes, BitConverter.GetBytes(IPAddress.HostToNetworkOrder(currentPID)), offset);

            offset = CopyInto(bytes, Storm.NetworkConstants.k_networkSeparator, offset);
            bytes[offset] = (byte)messageType;
            offset += 1;

            offset = CopyInto(bytes, Storm.NetworkConstants.k_networkSeparator, offset);
            offset = CopyInto(bytes, scriptContent, offset);

            offset = CopyInto(bytes, Storm.NetworkConstants.k_networkSeparator, offset);
            offset = CopyInto(bytes, Storm.NetworkConstants.k_endOfMessageCommand, offset);

            return bytes;
        }

        public static byte[] PrepareForSending(ScriptItemData scriptData, Int32 currentPID)
        {
            return PrepareForSending(scriptData._scriptContent, currentPID, Storm.NetworkMessageType.Script);
        }

        public static byte[] PreparePing(Int32 currentPID)
        {
            return PrepareForSending(string.Empty, currentPID, Storm.NetworkMessageType.Ping);
        }

        private static int CompareWithExpected(byte[] msg, int offset, string expected, out bool res)
        {
            res = true;

            int k_expectedLength = expected.Length;
            for (int iter = 0; iter < k_expectedLength; ++iter)
            {
                if (msg[iter + offset] != (byte)expected[iter])
                {
                    res = false;
                    break;
                }
            }

            return offset + k_expectedLength;
        }

        private static int CompareSeparator(byte[] msg, int offset, out bool res)
        {
            return CompareWithExpected(msg, offset, Storm.NetworkConstants.k_networkSeparator, out res);
        }

        public static bool AuthenticateConnection(Socket toConnect, Int32 currentPID)
        {
            int k_networkSepLength = Storm.NetworkConstants.k_networkSeparator.Length;

            int k_authenticationSize =
                4 + // magicKeyword
                1 + // application type
                4 + // PID
                8 + // version
                Storm.NetworkConstants.k_endOfMessageCommand.Length +
                k_networkSepLength * 4
                ;

            byte[] authenticationMsg = new byte[k_authenticationSize];

            try
            {
                toConnect.ReceiveTimeout = 50;
                toConnect.Receive(authenticationMsg, k_authenticationSize, SocketFlags.None);

                Int32 receivedMagicKeyword = IPAddress.NetworkToHostOrder(BitConverter.ToInt32(authenticationMsg, 0));
                if (receivedMagicKeyword != Storm.NetworkConstants.k_magicKeyword)
                {
                    return false;
                }

                bool isOk;
                int offset = CompareSeparator(authenticationMsg, 4, out isOk);
                if (!isOk)
                {
                    return false;
                }

                int applicationTypeOffIndex = offset;
                
                //BitConverter.ToChar(authenticationMsg, offset); // Application Type
                offset += 1;

                offset = CompareSeparator(authenticationMsg, offset, out isOk);
                if (!isOk)
                {
                    return false;
                }

                int pidOffIndex = offset;
                //BitConverter.ToChar(authenticationMsg, offset); // PID
                offset += 4;

                offset = CompareSeparator(authenticationMsg, offset, out isOk);
                if (!isOk)
                {
                    return false;
                }

                // Version has a fixed size, remnants are paddings.
                int offsetAndPadding = offset + 8;
                offset = CompareWithExpected(authenticationMsg, offset, Storm.NetworkConstants.k_networkVersion, out isOk);
                if (!isOk || offset > offsetAndPadding /*should not happen*/)
                {
                    return false;
                }
                for (; offset < offsetAndPadding; ++offset)
                {
                    if (authenticationMsg[offset] != '\0')
                    {
                        return false;
                    }
                }

                offset = CompareSeparator(authenticationMsg, offset, out isOk);
                if (!isOk)
                {
                    return false;
                }

                CompareWithExpected(authenticationMsg, offset, Storm.NetworkConstants.k_endOfMessageCommand, out isOk);
                if (!isOk)
                {
                    return false;
                }


                // Now we send authentication
                authenticationMsg[applicationTypeOffIndex] = (byte)Storm.NetworkApplication.Storm_ScriptSender;
                CopyInto(authenticationMsg, BitConverter.GetBytes(IPAddress.HostToNetworkOrder(currentPID)), pidOffIndex);
                
                toConnect.SendTimeout = 50;
                toConnect.Send(authenticationMsg);
            }
            catch (System.Exception)
            {
                return false;
            }

            return true;
        }
    }
}
