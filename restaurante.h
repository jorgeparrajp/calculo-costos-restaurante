#ifndef RESTAURANTE_H
#define RESTAURANTE_H

#define MAX_INGREDIENTES 100
#define MAX_PLATOS 100
#define MAX_RELACIONES 500
#define MAX_CODIGO 20
#define MAX_NOMBRE 60
#define MAX_TEXTO 40

typedef struct {
    char codigo[MAX_CODIGO];
    char nombre[MAX_NOMBRE];
    float costoUnitario;
    char unidadMedida[MAX_TEXTO];
} Ingrediente;

typedef struct {
    char codigo[MAX_CODIGO];
    char nombre[MAX_NOMBRE];
    char categoria[MAX_TEXTO];
    float impuesto;
    float servicio;
    float ganancia;
} Plato;

typedef struct {
    char codigoPlato[MAX_CODIGO];
    char codigoIngrediente[MAX_CODIGO];
    float cantidadUsada;
} PlatoIngrediente;

typedef struct {
    Ingrediente ingredientes[MAX_INGREDIENTES];
    Plato platos[MAX_PLATOS];
    PlatoIngrediente relaciones[MAX_RELACIONES];
    int totalIngredientes;
    int totalPlatos;
    int totalRelaciones;
} Sistema;

void inicializarSistema(Sistema *sistema);
void cargarDatos(Sistema *sistema);
void guardarDatos(const Sistema *sistema);

void menuIngredientes(Sistema *sistema);
void menuPlatos(Sistema *sistema);
void menuRelaciones(Sistema *sistema);
void menuReportes(const Sistema *sistema);

int leerEntero(const char *mensaje, int minimo, int maximo);
int confirmar(const char *mensaje);
void pausar(void);

#endif
