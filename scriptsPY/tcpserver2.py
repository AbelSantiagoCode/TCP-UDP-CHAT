#!/usr/bin/python
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

	def CloseServer(self):
		self.SERVER.close()

	def ServiceClient(self,Client):
		"""Prints Data sended by clients"""
		DataClient = Client.recv(1024)
		if not DataClient:
			self.CloseSockClient()
			self.CloseServer()
			sys.exit(1)
		print(DataClient)
		if (DataClient == "exit\n") or (DataClient == "exit"):
			self.RemoveSelect(Client)

	def Run(self):
		while True:
			rlist, wlist, xlist = select.select(self.SelectList,[],[])
			
			if rlist[0] == self.SelectList[0]:
				Client,Addr = self.SERVER.accept()   	#Accept Client 
				print("Connection from: "+str(Addr))
				self.SockClient.append(Client)
				self.AppendSelect(Client)

			elif rlist[0] != self.SelectList[1]: 		# CLIENT 
				ClientSock = rlist[0]
				self.ServiceClient(ClientSock)

			elif rlist[0] == self.SelectList[1]: 		# KEYBOARD -TERMINAL 
				DataKeyB = sys.stdin.readline()
				for ClientSock in self.SockClient:
					ClientSock.sendall(DataKeyB)
				if (DataKeyB == "exit") or (DataKeyB == "exit\n"):
					self.CloseSockClient()
					self.CloseServer()
					sys.exit(1)
			else:
				print("\nERROR")
		self.CloseSockClient()
		self.CloseServer()


if __name__ == '__main__':
	Server = ServerChat("",5000,5)
	Server.Run()
	
