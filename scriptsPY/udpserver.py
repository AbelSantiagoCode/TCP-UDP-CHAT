import socket

def Main():
	Host = '127.0.0.1'
	Port = 5000
	
	s = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
	s.bind((Host,Port))

	print("Server Started.")

	while True:
		""" rcv(buffer_size), buffer_size must be a pow of 2"""
		Data,Addr = Cliet.recvfrom(1024) 
		print("Message from: "+str(Addr))
		print("From connect user: "+str(Data))


		""" SERVICE OF SERVER: UPPERCASE"""
		print("Sending: "+ str(Data).upper())

		"""Send:
			
				socket.sendto(string, address)
				socket.sendto(string, flags, address): Send data to the socket. The socket should not be connected to a remote socket, since the destination socket is specified by address. The optional flags argument has the same meaning as for recv() above. Return the number of bytes sent. (The format of address depends on the address family — see above.)
		"""
		Cliet.sendto(str(Data).upper(),Addr)
	Cliet.close()

if __name__ == '__main__':
	Main()
