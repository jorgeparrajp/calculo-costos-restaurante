#include <stdio.h>
#include "restaurante.h"

static void mostrarMenuPrincipal(void) {
    printf("\n===== COSTOS DE RESTAURANTE =====\n");
    printf("1. Gestion de ingredientes\n");
    printf("2. Gestion de platos\n");
    printf("3. Ingredientes por plato\n");
    printf("4. Calculos y reportes\n");
    printf("5. Guardar cambios\n");
    printf("6. Salir\n");
}

int main(void) {
    enum { OPCION_MINIMA = 1, OPCION_SALIR = 6 };
    Sistema sistema;
    int opcion;

    inicializarSistema(&sistema);
    cargarDatos(&sistema);

    do {
        mostrarMenuPrincipal();

        opcion = leerEntero("Opcion: ", OPCION_MINIMA, OPCION_SALIR);

        switch (opcion) {
            case 1:
                menuIngredientes(&sistema);
                break;
            case 2:
                menuPlatos(&sistema);
                break;
            case 3:
                menuRelaciones(&sistema);
                break;
            case 4:
                menuReportes(&sistema);
                break;
            case 5:
                guardarDatos(&sistema);
                printf("Cambios guardados.\n");
                pausar();
                break;
            case OPCION_SALIR:
                if (confirmar("Desea guardar antes de salir")) {
                    guardarDatos(&sistema);
                }
                printf("Programa finalizado.\n");
                break;
        }
    } while (opcion != OPCION_SALIR);

    return 0;
}
