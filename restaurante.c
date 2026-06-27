#include "restaurante.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARCHIVO_INGREDIENTES "ingredientes.csv"
#define ARCHIVO_PLATOS "platos.csv"
#define ARCHIVO_RELACIONES "plato_ingredientes.csv"
#define MAX_LINEA 256

#define COSTO_MAXIMO 1000000.0f
#define PORCENTAJE_MAXIMO 99.0f

/* ---------- Entrada segura ---------- */

static void quitarSalto(char *texto) {
    texto[strcspn(texto, "\r\n")] = '\0';
}

static void leerTexto(const char *mensaje, char *destino, int tamano) {
    int caracter;

    while (1) {
        printf("%s", mensaje);

        if (fgets(destino, tamano, stdin) == NULL) {
            clearerr(stdin);
            continue;
        }

        if (strchr(destino, '\n') == NULL) {
            while ((caracter = getchar()) != '\n' && caracter != EOF) {
            }
        }

        quitarSalto(destino);

        if (destino[0] == '\0') {
            printf("La entrada no puede estar vacia.\n");
        } else if (strchr(destino, ',') != NULL) {
            printf("No use comas porque los datos se guardan en CSV.\n");
        } else {
            return;
        }
    }
}

int leerEntero(const char *mensaje, int minimo, int maximo) {
    char linea[80];
    char *fin;
    long valor;

    while (1) {
        printf("%s", mensaje);

        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }

        valor = strtol(linea, &fin, 10);
        while (isspace((unsigned char)*fin)) {
            fin++;
        }

        if (fin != linea && *fin == '\0' && valor >= minimo && valor <= maximo) {
            return (int)valor;
        }

        printf("Ingrese un numero entre %d y %d.\n", minimo, maximo);
    }
}

static float leerFloat(const char *mensaje, float minimo, float maximo, int incluyeMinimo) {
    char linea[80];
    char *fin;
    float valor;
    int minimoValido;

    while (1) {
        printf("%s", mensaje);

        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }

        valor = strtof(linea, &fin);
        while (isspace((unsigned char)*fin)) {
            fin++;
        }

        minimoValido = incluyeMinimo ? valor >= minimo : valor > minimo;

        if (fin != linea && *fin == '\0' && minimoValido && valor <= maximo) {
            return valor;
        }

        printf("Valor invalido.\n");
    }
}

int confirmar(const char *mensaje) {
    char respuesta[10];

    while (1) {
        printf("%s (s/n): ", mensaje);

        if (fgets(respuesta, sizeof(respuesta), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }

        if (respuesta[0] == 's' || respuesta[0] == 'S') {
            return 1;
        }
        if (respuesta[0] == 'n' || respuesta[0] == 'N') {
            return 0;
        }
    }
}

void pausar(void) {
    char linea[4];
    printf("Presione ENTER para continuar...");
    fgets(linea, sizeof(linea), stdin);
}

/* ---------- Busquedas ---------- */

static int contieneTexto(const char *texto, const char *busqueda) {
    char copiaTexto[MAX_NOMBRE];
    char copiaBusqueda[MAX_NOMBRE];
    int i;

    snprintf(copiaTexto, sizeof(copiaTexto), "%s", texto);
    snprintf(copiaBusqueda, sizeof(copiaBusqueda), "%s", busqueda);

    for (i = 0; copiaTexto[i] != '\0'; i++) {
        copiaTexto[i] = (char)tolower((unsigned char)copiaTexto[i]);
    }
    for (i = 0; copiaBusqueda[i] != '\0'; i++) {
        copiaBusqueda[i] = (char)tolower((unsigned char)copiaBusqueda[i]);
    }

    return strstr(copiaTexto, copiaBusqueda) != NULL;
}

static int buscarIngrediente(const Sistema *sistema, const char *codigo) {
    int i;
    for (i = 0; i < sistema->totalIngredientes; i++) {
        if (strcmp(sistema->ingredientes[i].codigo, codigo) == 0) {
            return i;
        }
    }
    return -1;
}

static int buscarPlato(const Sistema *sistema, const char *codigo) {
    int i;
    for (i = 0; i < sistema->totalPlatos; i++) {
        if (strcmp(sistema->platos[i].codigo, codigo) == 0) {
            return i;
        }
    }
    return -1;
}

static int buscarRelacion(const Sistema *sistema, const char *codigoPlato, const char *codigoIngrediente) {
    int i;
    for (i = 0; i < sistema->totalRelaciones; i++) {
        if (strcmp(sistema->relaciones[i].codigoPlato, codigoPlato) == 0 &&
            strcmp(sistema->relaciones[i].codigoIngrediente, codigoIngrediente) == 0) {
            return i;
        }
    }
    return -1;
}

static int ingredienteEnUso(const Sistema *sistema, const char *codigo) {
    int i;
    for (i = 0; i < sistema->totalRelaciones; i++) {
        if (strcmp(sistema->relaciones[i].codigoIngrediente, codigo) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ---------- Calculos ---------- */

static float costoIngredientes(const Sistema *sistema, const char *codigoPlato) {
    float total = 0.0f;
    int i;

    for (i = 0; i < sistema->totalRelaciones; i++) {
        if (strcmp(sistema->relaciones[i].codigoPlato, codigoPlato) == 0) {
            int pos = buscarIngrediente(sistema, sistema->relaciones[i].codigoIngrediente);
            if (pos >= 0) {
                total += sistema->ingredientes[pos].costoUnitario *
                         sistema->relaciones[i].cantidadUsada;
            }
        }
    }

    return total;
}

static float costoFinal(const Sistema *sistema, const Plato *plato) {
    float base = costoIngredientes(sistema, plato->codigo);
    float porcentajes = plato->impuesto + plato->servicio + plato->ganancia;
    return base + base * porcentajes / 100.0f;
}

/* ---------- Archivos CSV ---------- */

void inicializarSistema(Sistema *sistema) {
    memset(sistema, 0, sizeof(*sistema));
}

void cargarDatos(Sistema *sistema) {
    FILE *archivo;
    char linea[MAX_LINEA];

    archivo = fopen(ARCHIVO_INGREDIENTES, "r");
    if (archivo != NULL) {
        fgets(linea, sizeof(linea), archivo);
        while (sistema->totalIngredientes < MAX_INGREDIENTES &&
               fgets(linea, sizeof(linea), archivo) != NULL) {
            Ingrediente *x = &sistema->ingredientes[sistema->totalIngredientes];
            if (sscanf(linea, "%19[^,],%59[^,],%f,%39[^\r\n]",
                       x->codigo, x->nombre, &x->costoUnitario, x->unidadMedida) == 4 &&
                x->costoUnitario > 0) {
                sistema->totalIngredientes++;
            }
        }
        fclose(archivo);
    }

    archivo = fopen(ARCHIVO_PLATOS, "r");
    if (archivo != NULL) {
        fgets(linea, sizeof(linea), archivo);
        while (sistema->totalPlatos < MAX_PLATOS &&
               fgets(linea, sizeof(linea), archivo) != NULL) {
            Plato *x = &sistema->platos[sistema->totalPlatos];
            if (sscanf(linea, "%19[^,],%59[^,],%39[^,],%f,%f,%f",
                       x->codigo, x->nombre, x->categoria,
                       &x->impuesto, &x->servicio, &x->ganancia) == 6) {
                sistema->totalPlatos++;
            }
        }
        fclose(archivo);
    }

    archivo = fopen(ARCHIVO_RELACIONES, "r");
    if (archivo != NULL) {
        fgets(linea, sizeof(linea), archivo);
        while (sistema->totalRelaciones < MAX_RELACIONES &&
               fgets(linea, sizeof(linea), archivo) != NULL) {
            PlatoIngrediente *x = &sistema->relaciones[sistema->totalRelaciones];
            if (sscanf(linea, "%19[^,],%19[^,],%f",
                       x->codigoPlato, x->codigoIngrediente, &x->cantidadUsada) == 3 &&
                x->cantidadUsada > 0) {
                sistema->totalRelaciones++;
            }
        }
        fclose(archivo);
    }
}

void guardarDatos(const Sistema *sistema) {
    FILE *archivo;
    int i;

    archivo = fopen(ARCHIVO_INGREDIENTES, "w");
    if (archivo != NULL) {
        fprintf(archivo, "codigo_ing,nombre_ing,costo_unitario,unidad_medida\n");
        for (i = 0; i < sistema->totalIngredientes; i++) {
            fprintf(archivo, "%s,%s,%.2f,%s\n",
                    sistema->ingredientes[i].codigo,
                    sistema->ingredientes[i].nombre,
                    sistema->ingredientes[i].costoUnitario,
                    sistema->ingredientes[i].unidadMedida);
        }
        fclose(archivo);
    }

    archivo = fopen(ARCHIVO_PLATOS, "w");
    if (archivo != NULL) {
        fprintf(archivo, "codigo_plato,nombre_plato,categoria,impuesto_porcentaje,servicio_porcentaje,ganancia_porcentaje\n");
        for (i = 0; i < sistema->totalPlatos; i++) {
            fprintf(archivo, "%s,%s,%s,%.2f,%.2f,%.2f\n",
                    sistema->platos[i].codigo,
                    sistema->platos[i].nombre,
                    sistema->platos[i].categoria,
                    sistema->platos[i].impuesto,
                    sistema->platos[i].servicio,
                    sistema->platos[i].ganancia);
        }
        fclose(archivo);
    }

    archivo = fopen(ARCHIVO_RELACIONES, "w");
    if (archivo != NULL) {
        fprintf(archivo, "codigo_plato,codigo_ing,cantidad_usada\n");
        for (i = 0; i < sistema->totalRelaciones; i++) {
            fprintf(archivo, "%s,%s,%.2f\n",
                    sistema->relaciones[i].codigoPlato,
                    sistema->relaciones[i].codigoIngrediente,
                    sistema->relaciones[i].cantidadUsada);
        }
        fclose(archivo);
    }
}

/* ---------- Ingredientes ---------- */

static void registrarIngrediente(Sistema *sistema) {
    Ingrediente nuevo;

    if (sistema->totalIngredientes >= MAX_INGREDIENTES) {
        printf("Limite de ingredientes alcanzado.\n");
        return;
    }

    leerTexto("Codigo: ", nuevo.codigo, sizeof(nuevo.codigo));
    if (buscarIngrediente(sistema, nuevo.codigo) >= 0) {
        printf("Ese codigo ya existe.\n");
        return;
    }

    leerTexto("Nombre: ", nuevo.nombre, sizeof(nuevo.nombre));
    nuevo.costoUnitario = leerFloat("Costo unitario (> 0): ", 0, COSTO_MAXIMO, 0);
    leerTexto("Unidad de medida: ", nuevo.unidadMedida, sizeof(nuevo.unidadMedida));

    sistema->ingredientes[sistema->totalIngredientes++] = nuevo;
    printf("Ingrediente registrado.\n");
}

static void listarIngredientes(const Sistema *sistema) {
    int i;

    if (sistema->totalIngredientes == 0) {
        printf("No hay ingredientes.\n");
        return;
    }

    for (i = 0; i < sistema->totalIngredientes; i++) {
        printf("%s | %s | $%.2f | %s\n",
               sistema->ingredientes[i].codigo,
               sistema->ingredientes[i].nombre,
               sistema->ingredientes[i].costoUnitario,
               sistema->ingredientes[i].unidadMedida);
    }
}

static void buscarIngredienteMenu(const Sistema *sistema) {
    char busqueda[MAX_NOMBRE];
    int i;
    int encontrados = 0;

    leerTexto("Codigo o parte del nombre: ", busqueda, sizeof(busqueda));

    for (i = 0; i < sistema->totalIngredientes; i++) {
        if (strcmp(sistema->ingredientes[i].codigo, busqueda) == 0 ||
            contieneTexto(sistema->ingredientes[i].nombre, busqueda)) {
            printf("%s | %s | $%.2f | %s\n",
                   sistema->ingredientes[i].codigo,
                   sistema->ingredientes[i].nombre,
                   sistema->ingredientes[i].costoUnitario,
                   sistema->ingredientes[i].unidadMedida);
            encontrados++;
        }
    }

    if (encontrados == 0) {
        printf("No encontrado.\n");
    }
}

static void actualizarIngrediente(Sistema *sistema) {
    char codigo[MAX_CODIGO];
    int pos;

    leerTexto("Codigo del ingrediente: ", codigo, sizeof(codigo));
    pos = buscarIngrediente(sistema, codigo);

    if (pos < 0) {
        printf("Ingrediente no encontrado.\n");
        return;
    }

    leerTexto("Nuevo nombre: ", sistema->ingredientes[pos].nombre,
              sizeof(sistema->ingredientes[pos].nombre));
    sistema->ingredientes[pos].costoUnitario =
        leerFloat("Nuevo costo (> 0): ", 0, COSTO_MAXIMO, 0);
    leerTexto("Nueva unidad: ", sistema->ingredientes[pos].unidadMedida,
              sizeof(sistema->ingredientes[pos].unidadMedida));

    printf("Ingrediente actualizado.\n");
}

static void eliminarIngrediente(Sistema *sistema) {
    char codigo[MAX_CODIGO];
    int pos;
    int i;

    leerTexto("Codigo del ingrediente: ", codigo, sizeof(codigo));
    pos = buscarIngrediente(sistema, codigo);

    if (pos < 0) {
        printf("Ingrediente no encontrado.\n");
        return;
    }

    if (ingredienteEnUso(sistema, codigo)) {
        printf("No se puede eliminar porque esta asociado a un plato.\n");
        return;
    }

    if (!confirmar("Confirma la eliminacion")) {
        return;
    }

    for (i = pos; i < sistema->totalIngredientes - 1; i++) {
        sistema->ingredientes[i] = sistema->ingredientes[i + 1];
    }
    sistema->totalIngredientes--;

    printf("Ingrediente eliminado.\n");
}

void menuIngredientes(Sistema *sistema) {
    int opcion;

    do {
        printf("\n--- INGREDIENTES ---\n");
        printf("1. Registrar\n2. Listar\n3. Buscar\n4. Actualizar\n5. Eliminar\n0. Volver\n");
        opcion = leerEntero("Opcion: ", 0, 5);

        switch (opcion) {
            case 1: registrarIngrediente(sistema); break;
            case 2: listarIngredientes(sistema); break;
            case 3: buscarIngredienteMenu(sistema); break;
            case 4: actualizarIngrediente(sistema); break;
            case 5: eliminarIngrediente(sistema); break;
        }

        if (opcion != 0) {
            pausar();
        }
    } while (opcion != 0);
}

/* ---------- Platos ---------- */

static void registrarPlato(Sistema *sistema) {
    Plato nuevo;

    if (sistema->totalPlatos >= MAX_PLATOS) {
        printf("Limite de platos alcanzado.\n");
        return;
    }

    leerTexto("Codigo: ", nuevo.codigo, sizeof(nuevo.codigo));
    if (buscarPlato(sistema, nuevo.codigo) >= 0) {
        printf("Ese codigo ya existe.\n");
        return;
    }

    leerTexto("Nombre: ", nuevo.nombre, sizeof(nuevo.nombre));
    leerTexto("Categoria: ", nuevo.categoria, sizeof(nuevo.categoria));
    nuevo.impuesto = leerFloat("Impuesto (0 a 99): ", 0, PORCENTAJE_MAXIMO, 1);
    nuevo.servicio = leerFloat("Servicio (0 a 99): ", 0, PORCENTAJE_MAXIMO, 1);
    nuevo.ganancia = leerFloat("Ganancia (0 a 99): ", 0, PORCENTAJE_MAXIMO, 1);

    sistema->platos[sistema->totalPlatos++] = nuevo;
    printf("Plato registrado.\n");
}

static void mostrarPlato(const Sistema *sistema, int i) {
    float base = costoIngredientes(sistema, sistema->platos[i].codigo);

    printf("%s | %s | %s | Base: $%.2f | Final: $%.2f",
           sistema->platos[i].codigo,
           sistema->platos[i].nombre,
           sistema->platos[i].categoria,
           base,
           costoFinal(sistema, &sistema->platos[i]));

    if (base == 0) {
        printf(" | ALERTA: sin ingredientes");
    }
    printf("\n");
}

static void listarPlatos(const Sistema *sistema) {
    int i;

    if (sistema->totalPlatos == 0) {
        printf("No hay platos.\n");
        return;
    }

    for (i = 0; i < sistema->totalPlatos; i++) {
        mostrarPlato(sistema, i);
    }
}

static void buscarPlatoMenu(const Sistema *sistema) {
    char busqueda[MAX_NOMBRE];
    int i;
    int encontrados = 0;

    leerTexto("Codigo o parte del nombre: ", busqueda, sizeof(busqueda));

    for (i = 0; i < sistema->totalPlatos; i++) {
        if (strcmp(sistema->platos[i].codigo, busqueda) == 0 ||
            contieneTexto(sistema->platos[i].nombre, busqueda)) {
            mostrarPlato(sistema, i);
            encontrados++;
        }
    }

    if (encontrados == 0) {
        printf("No encontrado.\n");
    }
}

static void actualizarPlato(Sistema *sistema) {
    char codigo[MAX_CODIGO];
    int pos;

    leerTexto("Codigo del plato: ", codigo, sizeof(codigo));
    pos = buscarPlato(sistema, codigo);

    if (pos < 0) {
        printf("Plato no encontrado.\n");
        return;
    }

    leerTexto("Nuevo nombre: ", sistema->platos[pos].nombre,
              sizeof(sistema->platos[pos].nombre));
    leerTexto("Nueva categoria: ", sistema->platos[pos].categoria,
              sizeof(sistema->platos[pos].categoria));
    sistema->platos[pos].impuesto = leerFloat("Nuevo impuesto: ", 0, PORCENTAJE_MAXIMO, 1);
    sistema->platos[pos].servicio = leerFloat("Nuevo servicio: ", 0, PORCENTAJE_MAXIMO, 1);
    sistema->platos[pos].ganancia = leerFloat("Nueva ganancia: ", 0, PORCENTAJE_MAXIMO, 1);

    printf("Plato actualizado.\n");
}

static void eliminarPlato(Sistema *sistema) {
    char codigo[MAX_CODIGO];
    int pos;
    int i;

    leerTexto("Codigo del plato: ", codigo, sizeof(codigo));
    pos = buscarPlato(sistema, codigo);

    if (pos < 0) {
        printf("Plato no encontrado.\n");
        return;
    }

    if (!confirmar("Eliminar plato y sus relaciones")) {
        return;
    }

    i = 0;
    while (i < sistema->totalRelaciones) {
        if (strcmp(sistema->relaciones[i].codigoPlato, codigo) == 0) {
            int j;
            for (j = i; j < sistema->totalRelaciones - 1; j++) {
                sistema->relaciones[j] = sistema->relaciones[j + 1];
            }
            sistema->totalRelaciones--;
        } else {
            i++;
        }
    }

    for (i = pos; i < sistema->totalPlatos - 1; i++) {
        sistema->platos[i] = sistema->platos[i + 1];
    }
    sistema->totalPlatos--;

    printf("Plato eliminado.\n");
}

void menuPlatos(Sistema *sistema) {
    int opcion;

    do {
        printf("\n--- PLATOS ---\n");
        printf("1. Registrar\n2. Listar\n3. Buscar\n4. Actualizar\n5. Eliminar\n0. Volver\n");
        opcion = leerEntero("Opcion: ", 0, 5);

        switch (opcion) {
            case 1: registrarPlato(sistema); break;
            case 2: listarPlatos(sistema); break;
            case 3: buscarPlatoMenu(sistema); break;
            case 4: actualizarPlato(sistema); break;
            case 5: eliminarPlato(sistema); break;
        }

        if (opcion != 0) {
            pausar();
        }
    } while (opcion != 0);
}

/* ---------- Ingredientes por plato ---------- */

static void asociarIngrediente(Sistema *sistema) {
    PlatoIngrediente nueva;

    if (sistema->totalRelaciones >= MAX_RELACIONES) {
        printf("Limite de relaciones alcanzado.\n");
        return;
    }

    leerTexto("Codigo del plato: ", nueva.codigoPlato, sizeof(nueva.codigoPlato));
    if (buscarPlato(sistema, nueva.codigoPlato) < 0) {
        printf("El plato no existe.\n");
        return;
    }

    leerTexto("Codigo del ingrediente: ", nueva.codigoIngrediente,
              sizeof(nueva.codigoIngrediente));
    if (buscarIngrediente(sistema, nueva.codigoIngrediente) < 0) {
        printf("El ingrediente no existe.\n");
        return;
    }

    if (buscarRelacion(sistema, nueva.codigoPlato, nueva.codigoIngrediente) >= 0) {
        printf("La relacion ya existe.\n");
        return;
    }

    nueva.cantidadUsada = leerFloat("Cantidad usada (> 0): ", 0, COSTO_MAXIMO, 0);
    sistema->relaciones[sistema->totalRelaciones++] = nueva;

    printf("Ingrediente asociado al plato.\n");
}

static void listarIngredientesDePlato(const Sistema *sistema) {
    char codigoPlato[MAX_CODIGO];
    float total = 0.0f;
    int encontrados = 0;
    int i;

    leerTexto("Codigo del plato: ", codigoPlato, sizeof(codigoPlato));

    if (buscarPlato(sistema, codigoPlato) < 0) {
        printf("El plato no existe.\n");
        return;
    }

    for (i = 0; i < sistema->totalRelaciones; i++) {
        if (strcmp(sistema->relaciones[i].codigoPlato, codigoPlato) == 0) {
            int pos = buscarIngrediente(sistema, sistema->relaciones[i].codigoIngrediente);
            if (pos >= 0) {
                float parcial = sistema->ingredientes[pos].costoUnitario *
                                sistema->relaciones[i].cantidadUsada;

                printf("%s | %s | Cantidad: %.2f | Parcial: $%.2f\n",
                       sistema->ingredientes[pos].codigo,
                       sistema->ingredientes[pos].nombre,
                       sistema->relaciones[i].cantidadUsada,
                       parcial);

                total += parcial;
                encontrados++;
            }
        }
    }

    if (encontrados == 0) {
        printf("El plato no tiene ingredientes asociados.\n");
    } else {
        printf("Costo total de ingredientes: $%.2f\n", total);
    }
}

static void actualizarCantidad(Sistema *sistema) {
    char codigoPlato[MAX_CODIGO];
    char codigoIngrediente[MAX_CODIGO];
    int pos;

    leerTexto("Codigo del plato: ", codigoPlato, sizeof(codigoPlato));
    leerTexto("Codigo del ingrediente: ", codigoIngrediente, sizeof(codigoIngrediente));

    pos = buscarRelacion(sistema, codigoPlato, codigoIngrediente);
    if (pos < 0) {
        printf("La relacion no existe.\n");
        return;
    }

    sistema->relaciones[pos].cantidadUsada =
        leerFloat("Nueva cantidad (> 0): ", 0, COSTO_MAXIMO, 0);

    printf("Cantidad actualizada.\n");
}

static void eliminarRelacion(Sistema *sistema) {
    char codigoPlato[MAX_CODIGO];
    char codigoIngrediente[MAX_CODIGO];
    int pos;
    int i;

    leerTexto("Codigo del plato: ", codigoPlato, sizeof(codigoPlato));
    leerTexto("Codigo del ingrediente: ", codigoIngrediente, sizeof(codigoIngrediente));

    pos = buscarRelacion(sistema, codigoPlato, codigoIngrediente);
    if (pos < 0) {
        printf("La relacion no existe.\n");
        return;
    }

    if (!confirmar("Quitar ingrediente del plato")) {
        return;
    }

    for (i = pos; i < sistema->totalRelaciones - 1; i++) {
        sistema->relaciones[i] = sistema->relaciones[i + 1];
    }
    sistema->totalRelaciones--;

    printf("Relacion eliminada.\n");
}

void menuRelaciones(Sistema *sistema) {
    int opcion;

    do {
        printf("\n--- INGREDIENTES POR PLATO ---\n");
        printf("1. Asociar\n2. Listar\n3. Actualizar cantidad\n4. Quitar\n0. Volver\n");
        opcion = leerEntero("Opcion: ", 0, 4);

        switch (opcion) {
            case 1: asociarIngrediente(sistema); break;
            case 2: listarIngredientesDePlato(sistema); break;
            case 3: actualizarCantidad(sistema); break;
            case 4: eliminarRelacion(sistema); break;
        }

        if (opcion != 0) {
            pausar();
        }
    } while (opcion != 0);
}

/* ---------- Reportes ---------- */

static void detalleCosto(const Sistema *sistema) {
    char codigo[MAX_CODIGO];
    int pos;
    float base;
    float impuesto;
    float servicio;
    float ganancia;

    leerTexto("Codigo del plato: ", codigo, sizeof(codigo));
    pos = buscarPlato(sistema, codigo);

    if (pos < 0) {
        printf("Plato no encontrado.\n");
        return;
    }

    base = costoIngredientes(sistema, codigo);
    impuesto = base * sistema->platos[pos].impuesto / 100.0f;
    servicio = base * sistema->platos[pos].servicio / 100.0f;
    ganancia = base * sistema->platos[pos].ganancia / 100.0f;

    printf("\nPlato: %s\n", sistema->platos[pos].nombre);
    printf("Costo de ingredientes: $%.2f\n", base);
    printf("Impuesto: $%.2f\n", impuesto);
    printf("Servicio: $%.2f\n", servicio);
    printf("Ganancia: $%.2f\n", ganancia);
    printf("COSTO FINAL: $%.2f\n", base + impuesto + servicio + ganancia);
}

void menuReportes(const Sistema *sistema) {
    int opcion;

    do {
        printf("\n--- CALCULOS Y REPORTES ---\n");
        printf("1. Detalle de un plato\n2. Reporte general\n0. Volver\n");
        opcion = leerEntero("Opcion: ", 0, 2);

        switch (opcion) {
            case 1: detalleCosto(sistema); break;
            case 2: listarPlatos(sistema); break;
        }

        if (opcion != 0) {
            pausar();
        }
    } while (opcion != 0);
}
