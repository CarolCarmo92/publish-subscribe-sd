	#include "subscriptor.h"
	#include "comun.h"
	#include "edsu_comun.h"
	
	#include <stdio.h>
	#include <unistd.h>
	#include <stdlib.h>	
	#include <pthread.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <string.h>
	
	#define TAM 64
	
	int subscriptorIniciado = 0;
	void (*funcionNotificarEvento)(const char *, const char *);
	void (*funcionAltaTema)(const char *);
	void (*funcionBajaTema)(const char *);
	int puertoEscuchando;
	int socketIdent;
	
	int compruebaSiSubscriptorIniciado(){
		if(subscriptorIniciado){
		    return(1);
		}
		return 0;
	}	
		
	void * escuchaNotificacionesDeIntermediario(){
	  
		unsigned int tam_dir;
		struct sockaddr_in dir_cliente;

		
		while (1) {
			tam_dir=sizeof(dir_cliente);			
			int socket;			
			if ((socket=accept(socketIdent, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
				perror("error en accept");
				close(socket);
			}
			
			int leido;
			char buf[TAM];
			/* Lee el mensaje del socket -> buf */
			while ((leido=read(socket, buf, TAM))>0) {	
			
				/* Tratar notificacion A0=Tema A1=valor */
				
				char A0[TAM];
				char A1[TAM];	
				sscanf(buf,"%s %s",A0,A1);
				
				if(strcmp(A1,":alta")==0){
					if(funcionAltaTema != NULL){
						funcionAltaTema(A0);
					}
				}
				else if(strcmp(A1,":baja")==0){
					if(funcionBajaTema != NULL){
						funcionBajaTema(A0);
					}
				}
				else{
					funcionNotificarEvento(A0,A1);
				}
			}
			close(socket);
		}	
	}

	int alta_subscripcion_tema(const char *tema) {
	
		if(!compruebaSiSubscriptorIniciado()){
			 perror("subscriptor no iniciado\n");
			 return(-1);
		}
		
		char identificadorTipoMensaje[4]= "A";	
		
		char puertoParaRecibirNotificacion[10];
		sprintf(puertoParaRecibirNotificacion, "%d", puertoEscuchando);
		
		return enviaMensajeAIntermediario(tema, puertoParaRecibirNotificacion, identificadorTipoMensaje);
	}

	int baja_subscripcion_tema(const char *tema) {
	
		if(!compruebaSiSubscriptorIniciado()){
			 perror("subscriptor no iniciado\n");
			 return(-1);
		}		
		
		char identificadorTipoMensaje[4]= "B";		
		
		char puertoParaRecibirNotificacion[10];
		sprintf(puertoParaRecibirNotificacion, "%d", puertoEscuchando);		
		
		return enviaMensajeAIntermediario(tema, puertoParaRecibirNotificacion, identificadorTipoMensaje);
	}

	int inicio_subscriptor(void (*notif_evento)(const char *, const char *),
					void (*alta_tema)(const char *),
					void (*baja_tema)(const char *)) {
					
		funcionNotificarEvento=notif_evento;
		funcionAltaTema = alta_tema;
		funcionBajaTema = baja_tema;
		struct sockaddr_in dir;
		int opcion=1;		
		
		if ((socketIdent=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			perror("error creando socket");
		}

		/* Para reutilizar puerto inmediatamente */
		if (setsockopt(socketIdent, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion))<0){
			perror("error en setsockopt");
		}
		dir.sin_addr.s_addr=INADDR_ANY;
		dir.sin_family=PF_INET;
		
		if (bind(socketIdent, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
			perror("error en bind");
			close(socketIdent);	
		}
		
		/* Inicializa la variable global puertoEscuchando*/
		socklen_t len = sizeof(dir);
		getsockname(socketIdent, (struct sockaddr *)&dir, &len);
		puertoEscuchando = ntohs(dir.sin_port);	  
		
		if (listen(socketIdent, 5) < 0) {
			perror("error en listen");
			close(socketIdent);
		}
		/* Crea thread */
		pthread_t thid;
		pthread_attr_t atrib_th;
		pthread_attr_init(&atrib_th);
		pthread_attr_setdetachstate(&atrib_th, PTHREAD_CREATE_DETACHED);
		pthread_create(&thid, &atrib_th, escuchaNotificacionesDeIntermediario, NULL);

		subscriptorIniciado = 1;			
		return 0;
	}

	int fin_subscriptor() {
	
		if(!compruebaSiSubscriptorIniciado()){
			 perror("subscriptor no iniciado\n");
			 return(-1);
		}
		
		char identificadorTipoMensaje[4]= "F";		
				
		subscriptorIniciado = 0;
		
		return enviaMensajeAIntermediario(NULL, NULL, identificadorTipoMensaje);		
	}

