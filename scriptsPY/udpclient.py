import socket

def Main():
	host = '127.0.0.1'
	port = 5000
	server = (host,port)


	s = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
	#s.bind((host,port))

	message = raw_input(">>>")
	while message != "exit":
		s.sendto(message,server)
		data = s.recvfrom(1024)
		print("Received from server: "+str(data))
		message = raw_input(">>>")
	s.close()

if __name__ == '__main__':
	Main()