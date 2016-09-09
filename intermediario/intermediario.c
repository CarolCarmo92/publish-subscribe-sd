	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <string.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	#define TAM 64

	typedef struct nodeSubscriptores {
	  struct sockaddr_in dir_cliente;
	  struct nodeSubscriptores *next;
	} nodoSub;

	typedef struct node {
		struct node * next;
		struct nodeSubscriptores *subscriptores;
		char *nombre;
	} node_t;
	
	node_t * head = NULL;

	void print_list() {
		node_t * current = head;

		while (current != NULL) {
			printf("%s\n", current->nombre);
			current = current->next;
		}
	}
	
	void printSubList(node_t * tema) {
		nodoSub * current = tema->subscriptores;
		printf("imprimiendo subscriptores del tema %s\n", tema->nombre);
		while (current != NULL) {
			printf("%i %s\n", ntohs(current->dir_cliente.sin_port), inet_ntoa(current->dir_cliente.sin_addr));
			current = current->next;
		}
	}
	
	node_t* buscaTema(char *nombreTemaABuscar){
	
		node_t * current = head;
		while(current->next != NULL){
			if((0==strcmp(current->nombre, nombreTemaABuscar))){
				return (current);
			}
			current = current->next;
		}
		if((0==strcmp(current->nombre, nombreTemaABuscar))){
				return (current);
		}
		return(NULL);
	}
	
	int enviaMensajeNotificacion(struct sockaddr_in dirCliente, char *nombreTema, char *valor){
		/* 1: ok 0: mal */
		
		int s;		
		dirCliente.sin_family=PF_INET;	    
	    
		char mensaje[TAM]= "";
		strcat (mensaje, nombreTema);
		strcat (mensaje, " ");
		strcat (mensaje, valor);    

		if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			perror("error creando socket");
			return(0);
		}
		if (connect(s, (struct sockaddr *)&dirCliente, sizeof(dirCliente)) < 0) {
			perror("error en connect");
			close(s);
			return(0);
		}
		    
		if (write(s, mensaje, TAM)<0) {
			perror("error en write");
			close(s);
			return(0);
		}	
		shutdown (s, 2);
		return 1;
	}
	
	int enviaNotificaciones(char *nombreTema, char *valor){
		/* 1: ok 0: mal */
	  
		node_t * nodoTema = malloc(sizeof(node_t));			
		nodoTema = buscaTema(nombreTema);
		
		if(nodoTema == NULL){
			/* Tema no existe */
			return 0;
		}
		
		nodoSub * current = nodoTema->subscriptores;
		
		int valorRetorno = 1;

		while (current != NULL) {
			int enviado = 0;
			enviado = enviaMensajeNotificacion(current->dir_cliente, nombreTema, valor);
			if (enviado==0){
			      valorRetorno = 0;
			}
			current = current->next;
		}
		return (valorRetorno);
		
	}	

	/* Introduce nuevo tema en lista de temas */
	int nuevoTema(char *nombreTemaABuscar){
		/* 1:ok 0:mal */
		
		/* Comprueba que no existe */
		node_t * current;
		current = buscaTema(nombreTemaABuscar);
		
		if(current != NULL){
			return(0);	
		}		
		
		current = head;
		while (current->next != NULL) {
			current = current->next;
		}
		
		current->next = malloc(sizeof(node_t));
		
		current->next->nombre = strdup(nombreTemaABuscar);
		current->next->next = NULL;
		current->next->subscriptores = NULL;
		
		/* Notificar alta de tema a subscriptores */		
		nodoSub * actual = head->subscriptores;
		while (actual != NULL) {
			enviaMensajeNotificacion(actual->dir_cliente, nombreTemaABuscar, ":alta");
			actual = actual->next;
		}	
		
		return 1;
	}
	
	int nuevoSubscriptor(char *nombreTemaASubscribir, struct sockaddr_in dirCliente){
		/* 1:ok 0:mal */
		
		node_t * nodoTemaASuscribir = malloc(sizeof(node_t));			
		nodoTemaASuscribir = buscaTema(nombreTemaASubscribir);
			
		if(nodoTemaASuscribir == NULL){
			return 0;
		}
		
		nodoSub * nodoNuevoSubscriptor = malloc(sizeof(nodoSub));
		nodoNuevoSubscriptor->dir_cliente = dirCliente;
		nodoNuevoSubscriptor->next = NULL;		
		
		/* Introduce en lista de subs del dummy (lista general) */		
		int yaExiste = 0;
		if(head->subscriptores == NULL){
			head->subscriptores = nodoNuevoSubscriptor;
			
			node_t * iteradorTemas = head->next;

			while (iteradorTemas != NULL) {
				enviaMensajeNotificacion(dirCliente, iteradorTemas->nombre, ":alta");
				iteradorTemas = iteradorTemas->next;
			}
		}		 	
		else{	
		
			nodoSub * actual = head->subscriptores;
			while(actual!= NULL){
				if (actual->dir_cliente.sin_addr.s_addr == dirCliente.sin_addr.s_addr &&
				    actual->dir_cliente.sin_port == dirCliente.sin_port)
				{
					yaExiste =1;
				}
				actual = actual->next;
			}
			if(yaExiste==0){
			
				actual = nodoNuevoSubscriptor;
				node_t * iteradorTemas = head->next;

				while (iteradorTemas != NULL) {
					enviaMensajeNotificacion(dirCliente, iteradorTemas->nombre, ":alta");
					iteradorTemas = iteradorTemas->next;
				}
			}
		}
	
		if(nodoTemaASuscribir->subscriptores == NULL){
			/* Primer subscriptor */
			nodoTemaASuscribir->subscriptores = nodoNuevoSubscriptor;			
			return 1;
		}		  
	
		else{	
			/* Compruebo que no está ya subscrito al recorrer la lista*/
			nodoSub * current = nodoTemaASuscribir->subscriptores;
			while(current->next != NULL){
			  
				if (current->dir_cliente.sin_addr.s_addr == dirCliente.sin_addr.s_addr &&
				    current->dir_cliente.sin_port == dirCliente.sin_port)
				{					
					/* subscriptor ya existe */
					return 0;
				}
				current = current->next;
			}
			
			if (current->dir_cliente.sin_addr.s_addr == dirCliente.sin_addr.s_addr &&
				    current->dir_cliente.sin_port == dirCliente.sin_port)
			{
				/* subscriptor ya existe */
				return 0;
			}
			else{
				current->next = nodoNuevoSubscriptor;
				return 1;
			}
		}
	}	
	
	int eliminarTema(char *nombreTemaABuscar){
		/* 1:ok 0:mal */
	
		/* Comprueba que existe */
		node_t * current;
		current = buscaTema(nombreTemaABuscar);
		
		if(current == NULL){
			return(0);	
		}
		
		/* Notificar baja de tema a subscriptores */		
		nodoSub * actual = head->subscriptores;
		while (actual != NULL) {
			enviaMensajeNotificacion(actual->dir_cliente, nombreTemaABuscar, ":baja");
			actual = actual->next;
		}		
		
		current = head;
		node_t * beforeCurrent = head;
		while(current->next != NULL){
			if((0==strcmp(current->nombre, nombreTemaABuscar))){
				/* Elimina nodo intermedio con el tema */
				beforeCurrent->next = current->next;
				return (1);	
			}
			beforeCurrent = current;
			current = current->next;
		}
		beforeCurrent->next = NULL; /* Elimina ultimo nodo */	
		return (1);
	}
	
	int eliminarSubscriptor(char *nombreTemaAEliminar, struct sockaddr_in dirClienteAEliminar){
		/* 1:ok 0:mal */
		
		printf("print antes");
		printSubList(head);
		
		node_t * nodoTemaAEliminar = malloc(sizeof(node_t));	
		nodoTemaAEliminar = buscaTema(nombreTemaAEliminar);
			
		if(nodoTemaAEliminar == NULL){
			/* Tema no existe */
			return 0;
		}
		
		if(nodoTemaAEliminar->subscriptores == NULL){
			/* no hay subs de este tema */
			return 0;
		}

		nodoSub * current = nodoTemaAEliminar->subscriptores;
		nodoSub * beforeCurrent = nodoTemaAEliminar->subscriptores;
		
		if (current->dir_cliente.sin_addr.s_addr == dirClienteAEliminar.sin_addr.s_addr &&
				    current->dir_cliente.sin_port == dirClienteAEliminar.sin_port)
		{
			/* Elimina primer nodo */
			nodoTemaAEliminar->subscriptores = current->next;
			
			printf("print despues1");
			printSubList(head);
			
			return (1);	
		  
		}
		while(current->next != NULL){
			if (current->dir_cliente.sin_addr.s_addr == dirClienteAEliminar.sin_addr.s_addr &&
				    current->dir_cliente.sin_port == dirClienteAEliminar.sin_port)
				{
				/* Elimina nodo intermedio con el tema */
				beforeCurrent->next = current->next;
			
				printf("print despues2");
				printSubList(head);			
			
				return (1);	
			}
			beforeCurrent = current;
			current = current->next;
		}
		
		if (current->dir_cliente.sin_addr.s_addr == dirClienteAEliminar.sin_addr.s_addr &&
				    current->dir_cliente.sin_port == dirClienteAEliminar.sin_port)
		{
			beforeCurrent->next = NULL; /* Elimina ultimo nodo */
			
			printf("print despues3");
			printSubList(head);
			
			return (1);	
		}
		return 0;
		
	}
	
	int trataEvento(char buf[TAM], struct sockaddr_in dirCliente){
		/* 1: ok 0: mal */
			  
		char A0[TAM];
		char A1[TAM];
		char A2[TAM];		
		sscanf(buf,"%s %s %s",A0,A1,A2);
				
		if(A0[0] == 'A'){
			/*alta A2 = PUERTO  A1 = nombreTema */
			
			struct sockaddr_in dirClientePuertoMod;
			dirClientePuertoMod = dirCliente;
			dirClientePuertoMod.sin_port=htons(atoi(A2));
				
			return(nuevoSubscriptor(A1, dirClientePuertoMod));	
		}
		if(A0[0] == 'B'){
			/*baja A2 = PUERTO  A1 = nombreTema */
			
			struct sockaddr_in dirClientePuertoMod;
			dirClientePuertoMod = dirCliente;
			dirClientePuertoMod.sin_port=htons(atoi(A2));
			
			return(eliminarSubscriptor(A1, dirClientePuertoMod));	
		}
		
		if(A0[0] == 'F'){
			/* fin subscriptor */
			
			struct sockaddr_in dirClientePuertoMod;
			dirClientePuertoMod = dirCliente;
			dirClientePuertoMod.sin_port=htons(atoi(A2));
			
			int seHaEliminadoAlgo = 0;
			node_t * current = head;
			/* Recorre lista de temas */
			while (current != NULL) {
				if(eliminarSubscriptor(current->nombre, dirClientePuertoMod)){
					seHaEliminadoAlgo =1;
				}
				current = current->next;
			}
		
			return(seHaEliminadoAlgo);	
		}
		
		if(A0[0] == 'G'){
			/* generar tema A1 = tema  A2 = valor */
			return(enviaNotificaciones(A1,A2));		
		}
		if(A0[0] == 'C'){
			/*crear tema A1=nombreTema */		
			return(nuevoTema(A1));
					
		}
		if(A0[0] == 'E'){
			/*eliminar tema A1=nombreTema*/
			return(eliminarTema(A1));
		}
		return 0;
	}

	int main(int argc, char *argv[]) {
		
		if (argc!=3) {
			fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
			return 1;
		}
		
		/* inicializa la lista (primer valor no se usa) */
		
		head = malloc(sizeof(node_t));
		if (head == NULL) {
			return 1;
		}
		
		head->nombre = "(dummy)";
		head->next = NULL;
		head->subscriptores = NULL;
		
		/* lee fichero con temas */	
		FILE *fich;
		fich=fopen(argv[2],"r");
		if(fich==NULL)
		{
			perror("error lectura fichero");
			return 0;
		}
			
		/* Agrega temas leidos a la lista */
		char linea[TAM];
		while(fgets(linea,TAM,fich) != NULL){
			char *temaLeido = malloc(sizeof(char[TAM]));
			sscanf (linea, "%s", temaLeido);
			nuevoTema(temaLeido);
		}
		
		int s, s_conec;
		unsigned int tam_dir;
		struct sockaddr_in dir, dir_cliente;
		int opcion=1;

		if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			perror("error creando socket");
			return 1;
		}

		/* Para reutilizar puerto inmediatamente */
		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion))<0){
			perror("error en setsockopt");
			return 1;
		}
		dir.sin_addr.s_addr=INADDR_ANY;
		dir.sin_port=htons(atoi(argv[1]));
		dir.sin_family=PF_INET;
		
		if (bind(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
			perror("error en bind");
			close(s);
			return 1;
		}
	  
		if (listen(s, 10) < 0) {
			perror("error en listen");
			close(s);
			return 1;
		}

		while (1) {
			
			tam_dir=sizeof(dir_cliente);
			if ((s_conec=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
				perror("error en accept");
				close(s);
				return 1;
			}
		
			int leido;
			char buf[TAM];
			/* Lee el mensaje del socket -> buf */
			while ((leido=read(s_conec, buf, TAM))>0) {		  
				int eventoCorrecto = trataEvento(buf, dir_cliente);				
				if(eventoCorrecto){
					/* Respuesta de confirmacion (ok) */
					char ok[2] = "ok";
					if (write(s_conec, ok, 2)<0) {	      
						perror("error en write");
						close(s);
						close(s_conec);
						return 1;
					}
				}
				else{
					/* Respuesta de confirmacion (no) */
					char no[2] = "no";
					if (write(s_conec, no, 2)<0) {	      
						perror("error en write");
						close(s);
						close(s_conec);
						return 1;
					}
				}
			}
		
			if (leido<0) {
				perror("error en read");
				close(s);
				close(s_conec);
				return 1;
			}
			close(s_conec);
		}

		close(s);
		return 0;
	}