import socket 
import select
import sys

def ClientChat(HOST,PORT): # The remote host ; The same port as used by the server

	try:
		Client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		Client.connect((HOST,PORT))
	except socket.error, e:
		print("Error creating socket: "+str(e))
		sys.exit(1)		
	
	SelectList = [Client,sys.stdin]

	while True:
		try :
			rlist, wlist, xlist = select.select(SelectList,[],[])
			if rlist[0] == SelectList[0]: 
				DataServer = Client.recv(1024)
				if not DataServer: break		
				print(DataServer)
				if (DataServer == "exit\n") or (DataServer == "exit"):
					Client.close()
					sys.exit(1)	

			elif rlist[0] == SelectList[1]:
				DataKeyB = sys.stdin.readline()
				Client.sendall(DataKeyB)
				if (DataKeyB == "exit") or (DataKeyB == "exit\n"):
					Client.close()
					sys.exit(1)		
			else :
				print("Error")

		except Exception as e:
			Client.close()
			sys.exit(1)
		except KeyboardInterrupt :
			print("data KeyboardInterrupt")
			Client.sendall("exit\n")
			Client.close()
			sys.exit(1)


if __name__ == '__main__':
	ClientChat('127.0.0.1',5000)



