import socket
import select 
import sys


def Main():
	Host = ''            # Symbolic name meaning all available interfaces #IP DEL SERVIDOR-PC exemple: '172.20.14.5', '' -> agafa la ip del localhost
	Port = 5000          # Arbitrary non-privileged port
	
	SERVER = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	SERVER.bind((Host,Port))
	SERVER.listen(2)
	Client,Addr = SERVER.accept()
	print("Connection from: "+str(Addr))
	file_d = [Client, sys.stdin]

	while True:
		rlist, wlist, xlist = select.select(file_d,[],[])

		if rlist[0] == file_d[0]:
			try:
				DataClient = Client.recv(1024) 
				if not DataClient: break
				print(DataClient)

			except Exception as e:
				print("Connection finished")
				Client.close()
				SERVER.close()
				break

		elif rlist[0] == file_d[1] :
			DataKeyB = sys.stdin.readline()
			Client.sendall(DataKeyB)

		else :
			print("ERROR")
	Client.close()
	SERVER.close()


if __name__ == '__main__':
	Main()


