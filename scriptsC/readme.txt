		APLICACIÓ SERVIDOR:
======================================================


CODI SERVER1.C
###############
El codi server1.c implementa un servidor de protocol TCP, el qual permet només un client. Defineix la ip del servidor en la variable **char ip[18]="127.0.0.1"**.

El programa s'executa : ./server1 port

El programa principal permet un chat bidireccional.




CODI SERVER2.C
###############

El codi server2.c implementa un servidor de protocol TCP, el qual permet multiples clients amb la funció select(). Defineix la ip del servidor en la variable **char ip[18]="127.0.0.1"**.

El programa s'executa : ./server2 port


El programa principal permet un chat entre clients i servidor, on el missatge del servidor s'envia a tots el clients actius i els missatges dels clients es mostren pel terminal corresponent al servidor. Si un client vol sortir del chat, ha d'enviar "exit" i el servidor segueix oferint servei als clients restants, respectivament si el servidor envia "exit" el chat s'acaba. 


CODI SERVER3.C
###############

El codi server3.c implementa un servidor de protocol TCP, el qual permet multiples clients amb un procès per client. Defineix la ip del servidor en la variable **char ip[18]="127.0.0.1"**.

El programa s'executa : ./server3 port        
EXEMPLE: ./server3 5000


El programa principal permet un chat entre clients i servidor, on el missatge del servidor s'envia a tots el clients actius i els missatges dels clients es mostren pel terminal corresponent al servidor. Si un client vol sortir del chat, ha d'enviar "exit" i el servidor segueix oferint servei als clients restants, respectivament si el servidor envia "exit" el chat s'acaba. 



		APLICACIÓ CLIENT:
======================================================

CODI CLIENT.C
###############

El codi client.c implementa un client de protocol TCP.

El programa s'executa: ./client ip-server port-server
EXEMPLE: ./client 127.0.0.1 5000

El programa principal permet un chat amb el servidor. Si el Client vol sortir del chat ha d'envia "exit".



