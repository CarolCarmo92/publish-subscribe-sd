/*
   Incluya en este fichero todas las implementaciones que pueden
   necesitar compartir los módulos editor y subscriptor,
   si es que las hubiera.
*/

#include "edsu_comun.h"

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <stdio.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <string.h>

    #define TAM 64


int enviaMensajeAIntermediario(const char *tema,const char *valor, char *identificadorTipoMensaje){
	
		int s;
		struct sockaddr_in dir;
		struct hostent *host_info;

		char *host=getenv("SERVIDOR");
		host_info=gethostbyname(host);
		memcpy(&dir.sin_addr.s_addr, host_info->h_addr, host_info->h_length);
		
		char *puertoC=getenv("PUERTO");
		int puerto=atoi(puertoC);
		dir.sin_port=htons(puerto);	
		
		dir.sin_family=PF_INET;	    
	    
		char mensaje[TAM]= "";
		if(valor != NULL){
			strcat (mensaje, identificadorTipoMensaje);
			strcat (mensaje, " ");
			strcat (mensaje, tema);
			strcat (mensaje, " ");
			strcat (mensaje, valor);
		}
		
		else{
			strcat (mensaje, identificadorTipoMensaje);
			strcat (mensaje, " ");
			strcat (mensaje, tema);
		}	    

	    if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		    perror("error creando socket");
		    return(-1);
	    }
	    if (connect(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
		    perror("error en connect");
		    close(s);
		    return(-1);
	    }
                
	    if (write(s, mensaje, TAM)<0) {
		    perror("error en write");
		    close(s);
		    return(-1);
	    }	
	    
	    int leido;
	    char buf[TAM];	    
	    if ((leido=read(s, buf, TAM))<0) {
                    perror("error en read");
                    close(s);
                    return (-1);
            }
	    
	    int res;	    
	    if( (res = strncmp("ok", buf, 2)) != 0){
		    printf("error: peticion invalida");
		    close(s);
		    return(-1);
	    }
		    
	    close(s);	
		return 0;
	}