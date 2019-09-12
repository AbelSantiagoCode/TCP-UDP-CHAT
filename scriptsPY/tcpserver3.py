#!/usr/bin/python
from multiprocessing import Process
import socket
import select 
import sys

class ServerChat(object):
	"""ServerChat which uses TCP protocol"""
	def __init__(self, HOST, PORT,NClient):
		self.HOST = HOST
		self.PORT = PORT
		self.NClient    = NClient
		self.SockClient = []
		self.ProClient 	= []
		try:
			self.SERVER  = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			self.SERVER.bind((HOST,PORT))
			self.SERVER.listen(NClient)
		except socket.error, e:
			print("Error creating socket: "+str(e))
			sys.exit(1)		
		self.SelectList = [self.SERVER,sys.stdin]


	def AppendSelect(self,Arg):
		self.SelectList.append(Arg)

	def RemoveSelect(self,Arg):
		self.SelectList.remove(Arg)

	def CloseSockClient(self):
		for Client in self.SockClient:
			Client.close()
	
	def CloseProClient(self):
		for PChild in self.ProClient:
			PChild.terminate()

	def CloseServer(self):
		self.SERVER.close()

	def ServiceClient(self,Client):
		"""Prints Data sended by clients"""
		while True:
			DataClient = Client.recv(1024)
			if not DataClient: break
			print(DataClient)


	def ClientProcess(self,Client):
		try:
			ClientPro = Process(target = self.ServiceClient, args=(Client,))  # Delegate a Process to each Client
			ClientPro.daemon = True # Automatically the child process deads before his father terminates 
			ClientPro.start()
			self.ProClient.append(ClientPro)    # Append Process-Client to a list

		except Exception as e:
			ClientPro.terminate()
			print("\nProcess can't create\n")

		

	def Run(self):
		while True:
			rlist, wlist, xlist = select.select(self.SelectList,[],[])
			
			if rlist[0] == self.SelectList[0]:
				Client,Addr = self.SERVER.accept()   	#Accept Client 
				print("Connection from: "+str(Addr))
				self.ClientProcess(Client)
				self.SockClient.append(Client)

			elif rlist[0] == self.SelectList[1]:		#KEYBOARD-TERMINAL
				DataKeyB = sys.stdin.readline()
				for ClientSock in self.SockClient:
					ClientSock.sendall(DataKeyB)
				if DataKeyB == "exit\n":
					self.CloseSockClient()
					self.CloseProClient()
					self.CloseServer()


			else:
				print("\nERROR")
		self.CloseSockClient()
		self.CloseProClient()
		self.CloseServer()


if __name__ == '__main__':
	Server = ServerChat("",5000,1) # ServerChat(IpServer,Port,Nombre de Clients)
	Server.Run()
