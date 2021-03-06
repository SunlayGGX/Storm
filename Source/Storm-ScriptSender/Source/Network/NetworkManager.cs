﻿using Storm_ScriptSender.Source.General.Config;
using Storm_ScriptSender.Source.Script;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace Storm_ScriptSender.Source.Network
{
    public class NetworkManager
    {
        #region Member
        private TcpListener _netListener = null;
        private List<Socket> _clients = new List<Socket>();

        private List<ScriptItemData> _backToSend = new List<ScriptItemData>();
        private List<ScriptItemData> _frontToSend = new List<ScriptItemData>();

        private List<Socket> _closedToRemove = new List<Socket>();

        private Thread _networkThread = null;

        private bool _running = true;

        private Task<Socket> _acceptance = null;

        private Int32 _pid;

        private CancellationTokenSource _acceptanceWaitCancellationToken = new CancellationTokenSource();

        DateTime _nextPingTime = DateTime.MinValue;


        private static NetworkManager s_instance = null;
        public static NetworkManager Instance
        {
            get
            {
                if (s_instance == null)
                {
                    s_instance = new NetworkManager();
                }

                return s_instance;
            }
        }


        private bool _lastConnectionState = false;
        public delegate void OnConnectionStateChangedHandler(bool connected);
        public event OnConnectionStateChangedHandler OnConnectionStateChanged;

        #endregion


        #region Method

        public void Initialize()
        {
            _pid = Process.GetCurrentProcess().Id;
            _networkThread = new Thread(() => this.Run());
            _networkThread.Start();
        }

        private void Run()
        {
            IPAddress localhostAddress = IPAddress.Parse("127.0.0.1");
            _netListener = new TcpListener(localhostAddress, (int)ConfigManager.Instance.Port);
            _netListener.Start();

            while (_running)
            {
                if (this.TestOrTryConnection())
                {
                    this.SwapDoubleBuffer();

                    if (_frontToSend.Count > 0)
                    {
                        foreach(ScriptItemData scriptDataToSend in _frontToSend)
                        {
                            byte[] bytesToSend = NetworkHelpers.PrepareForSending(scriptDataToSend, _pid);
                            foreach (Socket client in _clients)
                            {
                                if (_running)
                                {
                                    SendThroughSocket(client, bytesToSend);
                                }
                                else
                                {
                                    return;
                                }
                            }

                            if (_closedToRemove.Count > 0)
                            {
                                foreach (Socket toClose in _closedToRemove)
                                {
                                    toClose.Close();
                                    _clients.Remove(toClose);
                                }
                                _closedToRemove.Clear();
                            }
                        }

                        _frontToSend.Clear();
                    }
                }
                else if (_running)
                {
                    try
                    {
                        // Additional pause while waiting for any connection.
                        if (_acceptance?.Wait(250, _acceptanceWaitCancellationToken.Token) ?? false)
                        {
                            AddConnection(_acceptance.Result);
                            _acceptance = null;
                        }
                    }
                    catch (ObjectDisposedException)
                    {
                        if (!_running)
                        {
                            return;
                        }

                        _acceptance = null;

                        Console.WriteLine("Error when adding connection : Cannot access a disposed objects (here a TcpSocket).");
                    }
                    catch (OperationCanceledException)
                    {
                        if (!_running)
                        {
                            return;
                        }
                        
                        _acceptance = null;

                        Console.WriteLine("Error when adding connection : Connection Canceled.");
                    }
                }
                else
                {
                    return;
                }

                if (_clients.Count > 0)
                {
                    if (DateTime.Now > _nextPingTime)
                    {
                        _nextPingTime = DateTime.Now.AddMilliseconds(2500);
                        byte[] pingMessage = NetworkHelpers.PreparePing(_pid);
                        foreach (Socket socket in _clients)
                        {
                            try
                            {
                                socket.Send(pingMessage);
                            }
                            catch (SocketException)
                            {
                                _closedToRemove.Add(socket);
                            }
                        }

                        if (_closedToRemove.Count > 0)
                        {
                            foreach (Socket toClose in _closedToRemove)
                            {
                                toClose.Close();
                                _clients.Remove(toClose);
                            }
                            _closedToRemove.Clear();
                        }
                    }
                }

                if (_running)
                {
                    Thread.Sleep(100);
                }
                else
                {
                    return;
                }
            }
        }

        public void Close()
        {
            _running = false;
            lock (_backToSend)
            {
                _backToSend.Clear();
            }

            _acceptanceWaitCancellationToken.Cancel();
            Thread.Sleep(10);

            _netListener.Stop();
            _acceptance = null;

            _networkThread.Join();

            foreach (Socket toClose in _clients)
            {
                toClose.Close();
            }
            foreach (Socket toClose in _closedToRemove)
            {
                toClose.Close();
            }

            _clients.Clear();
            _closedToRemove.Clear();
        }

        private void SwapDoubleBuffer()
        {
            List<ScriptItemData> tmp;
            lock (_backToSend)
            {
                tmp = _backToSend;
                _backToSend = _frontToSend;
                _frontToSend = tmp;
            }
        }

        private bool TestOrTryConnection()
        {
            try
            {
                if (_acceptance?.IsCompleted ?? false)
                {
                    this.AddConnection(_acceptance.Result);
                }

                while (_acceptance == null)
                {
                    Console.WriteLine("Listening to new connections");

                    _acceptance = _netListener.AcceptSocketAsync();
                    if (_acceptance.Wait(10, _acceptanceWaitCancellationToken.Token))
                    {
                        this.AddConnection(_acceptance.Result);
                        _acceptance = null;
                    }
                }
            }
            catch (ObjectDisposedException)
            {
                if (!_running)
                {
                    return false;
                }

                _acceptance = null;
            }
            catch (OperationCanceledException)
            {
                if (!_running)
                {
                    return false;
                }

                _acceptance = null;
            }

            foreach (Socket client in _clients)
            {
                if (!this.SocketIsConnected(client))
                {
                    _closedToRemove.Add(client);
                }
            }

            if (_closedToRemove.Count > 0)
            {
                foreach (Socket toClose in _closedToRemove)
                {
                    toClose.Close();
                    _clients.Remove(toClose);
                }
                _closedToRemove.Clear();
            }

            if (_clients.Count > 0)
            {
                try
                {
                    if (!_lastConnectionState)
                    {
                        _lastConnectionState = true;
                        OnConnectionStateChanged?.Invoke(true);
                    }
                }
                catch (Exception)
                {

                }

                return true;
            }
            else
            {
                try
                {
                    if (_lastConnectionState)
                    {
                        _lastConnectionState = false;
                        OnConnectionStateChanged?.Invoke(false);
                    }
                }
                catch (Exception)
                {

                }

                return false;
            }
        }

        private void SendThroughSocket(Socket socket, byte[] bytesToSend)
        {
            try
            {
                int totalSend = 0;
                
                do
                {
                    totalSend += socket.Send(bytesToSend, totalSend, bytesToSend.Length - totalSend, SocketFlags.None);
                } while (totalSend != bytesToSend.Length);

                Console.WriteLine("Message sent to " + socket.RemoteEndPoint.ToString());
            }
            catch (SocketException)
            {
                _closedToRemove.Add(socket);
            }
            catch (ObjectDisposedException)
            {
                _closedToRemove.Add(socket);
            }
        }

        public void SendScript(ScriptItem script)
        {
            if (script.ScriptTextContent.Length > 0)
            {
                ScriptItemData data = new ScriptItemData()
                {
                    _scriptContent = script.ScriptTextContent
                };

                lock (_backToSend)
                {
                    _backToSend.Add(data);
                }
            }
        }

        private void AddConnection(Socket toConnect)
        {
            string remoteEndPoint = toConnect.RemoteEndPoint.ToString();
            Console.WriteLine(remoteEndPoint + " is connecting");

            if (NetworkHelpers.AuthenticateConnection(toConnect, _pid))
            {
                _clients.Add(toConnect);
                Console.WriteLine(remoteEndPoint + " authenticated successfully");
            }
            else
            {
                toConnect.Close();
            }
        }

        private class BlockingGuard : IDisposable
        {
            bool _blockingState;
            Socket _socket = null;

            public BlockingGuard(Socket socket)
            {
                _blockingState = socket.Blocking;
                _socket = socket;
            }

            public void Dispose()
            {
                if (_socket != null)
                {
                    _socket.Blocking = _blockingState;
                }
            }
        }

        private bool SocketIsConnected(Socket client)
        {
            return client.Connected;
        }

        #endregion
    }
}
