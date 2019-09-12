		APLICACIÓ SERVIDOR:
======================================================


CODI TCPSERVER1.PY
##################
El codi tcpsrver1.py implementa un servidor de protocol TCP, el qual permet només un client. 

El programa s'executa : python tcpserver1.py

El programa principal permet un chat bidireccional amb el client.




CODI TCPSERVER2.PY
##################

El codi tcpserver2.py implementa un servidor de protocol TCP, el qual permet multiples clients amb la funció select(). 

La configuració del la IP del Servidor,PORT,i nombre de clients es fa en el moment de fer la instància de la classe **ServerChat(IP,PORT,NCLIENTS)**. És necessari fer la crida al mètode Run() per activar el chat.

EXEMPLE:
	Server = ServerChat("",5000,5)
	Server.Run()


El programa s'executa : python tcpserver2.py


El programa principal permet un chat entre clients i servidor, on el missatge del servidor s'envia a tots el clients actius i els missatges dels clients es mostren pel terminal corresponent al servidor. Si un client vol sortir del chat, ha d'enviar "exit" i el servidor segueix oferint servei als clients restants, respectivament si el servidor envia "exit" el chat s'acaba. 


CODI TCPSERVER3.PY
##################

El codi tcpserver3.py implementa un servidor de protocol TCP, el qual permet multiples clients amb un procès per client.


La configuració del la IP del Servidor,PORT,i nombre de clients es fa en el moment de fer la instància de la classe **ServerChat(IP,PORT,NCLIENTS)**. És necessari fer la crida al mètode Run() per activar el chat. 

EXEMPLE:
	Server = ServerChat("",5000,5)
	Server.Run()


El programa s'executa : python tcpserver3.py        



El programa principal permet un chat entre clients i servidor, on el missatge del servidor s'envia a tots el clients actius i els missatges dels clients es mostren pel terminal corresponent al servidor. 

NOTA: en aquest programa nomes el servidor por fet "exit" per acabar amb el programa, no hi ha agut temps per implementar la sortida dels clients. Per tant si un client envia EXIT, es tractara com un missatge normal mostran-lo per pantalla del servidor.


		APLICACIÓ CLIENT:
======================================================

CODI TCPCLIENT.PY
#################

El codi tcpclient.py implementa un client de protocol TCP.

El programa s'executa: python tcpclient.py


El programa principal permet un chat amb el servidor. Si el Client vol sortir del chat ha d'envia "exit".





	FITXERS UPDCLIENT.PY I UCPSERVER.PY
=========================================================
Representen un chat de tipus un servidor un client simple amb el protocol UDP, tot i que en aquest cas la comunicació del chat va per torns degut a la funció bloquejant de **raw_input()**








