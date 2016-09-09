    #include "editor.h"
    #include "comun.h"
    #include "edsu_comun.h"

    #include <stdio.h>
    #include <unistd.h>
    #include <stdlib.h>

    #define TAM 64

    int generar_evento(const char *tema, const char *valor) {
	
	    char identificadorTipoMensaje[4]= "G";
		
	    return enviaMensajeAIntermediario(tema,valor, identificadorTipoMensaje);
    }

    /* solo para la version avanzada */
    int crear_tema(const char *tema) {
	    char identificadorTipoMensaje[4]= "C";
		
	    return enviaMensajeAIntermediario(tema,NULL, identificadorTipoMensaje);
    }

    /* solo para la version avanzada */
    int eliminar_tema(const char *tema) {
	    char identificadorTipoMensaje[4]= "E";
		
	    return enviaMensajeAIntermediario(tema,NULL, identificadorTipoMensaje);
    }



