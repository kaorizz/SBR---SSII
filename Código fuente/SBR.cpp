#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <string>
#include <math.h>
#include <list>

#define REGLA_Y 0    //Operador y
#define REGLA_O 1    //Operador o
#define REGLA_C 2    //Solo un antecedente

using namespace std;

struct regla   //Estructura de una regla
{
    double factorCert;  //Factor de certeza de la regla
    int tipo = REGLA_C;  //Por defecto el tipo de una regla es REGLA_C
    string consec, id;
    list<string> antec;  //Lista de antecedentes de la regla
};

struct hecho   //Estructura de un hecho
{
    double factorCert;  //Factor de certeza de un hecho
    string h;   //Identificador del hecho
};

ofstream ficSalida;    //Fichero de salida
list<hecho> baseHechos; //Base de hechos en forma de lista
list<regla> baseConocimiento; //Base de conocimiento en forma de lista
hecho *nuevoHecho;  //Puntero que nos sirve para crear nuevos hechos
regla *nuevaRegla;  //Puntero que nos sirve para crear nuevas reglas
list<hecho>::iterator itHechos;  //Iterador de la lista de hechos

void print_error(int error)
{
    switch(error)
    {
        case 0:
            cout << "Error: Número de argumentos incorrecto (tiene que ser 3)" << endl;
            exit(1);
        case 1:
            cout << "Error: No se pudo abrir el fichero correspondiente a la BC" << endl;
            exit(1);
        case 2:
            cout << "Error: No se pudo abrir el fichero correspondiente a la BH" << endl;
            exit(1);
    }
}

void actualizarBH(string h, double fc){ //Funcion para actualizar la base de hechos
    //Reservamos memoria para un nuevo hecho y lo añadimos a la base de hechos
    nuevoHecho = new hecho;
    nuevoHecho->h = h;
    nuevoHecho->factorCert = fc;
    baseHechos.push_back(*nuevoHecho);  //Añadimos el nuevo hecho a la lista
}

double buscarObjetivo(string objetivo)  //Funcion para saber si el objetivo esta contenido en la base de hechos
{     
    itHechos = baseHechos.begin();
    while (itHechos!=baseHechos.end())
    {
        if(itHechos->h == objetivo)
         return itHechos->factorCert; //Devolvemos el factor de certeza del hecho
        itHechos++;
    }
    return -2;  //Devolvemos -2 si el objetivo no se encuentra en la base de hechos
}

string factorizarNombre(string arg) //Función que se encarga de factorizar el nombre del archivo que se le pasa (quitar el .txt)
{
    string::iterator it;    //Creamos un iterador de strings para obtener las subcandenas que nos interesan
    string nombreResul;
    it = arg.end();
    it--;
    while (it!=arg.begin()-1)
    {
        if(*it == '.')
         nombreResul.clear();
        else if(*it == '\\')
         break;
        else nombreResul = *it + nombreResul;
        it--;
    }
    return nombreResul;
}

string concatenarFicheros(string arg1, string arg2) //Función correspondiente a concatenar el nombre de los ficheros que se pasan
{
    string nombreConcatenado;
    nombreConcatenado = arg1;   //Añadimos el nombre de la BC
    nombreConcatenado += arg2;  //Añadimos el nombre de la BH
    nombreConcatenado += ".txt";   //Añadimos la extensión .txt al fichero final
    return nombreConcatenado; 
}

double motInferencia(string objetivo)   //Función correspondiente al motor de inferencia
{
    double obj = buscarObjetivo(objetivo);

    if(obj != -2)
     return obj;   //Si el objetivo está contenido en la base de hechos devolvemos su valor

    list<double> fcLista;    //En esta lista guardamos los factores de certeza del objetivo para cada una de las reglas
    string reglas, operador;
    list<regla>::iterator itReglas;
    list<string>::iterator it;

    itReglas = baseConocimiento.begin();

    while (itReglas!=baseConocimiento.end()) //Vamos evaluando la base de conocimientos hasta que encontramos una regla que tiene como consecuente el objetivo
    {
        if (itReglas->consec == objetivo) //Si tiene el objetivo como consecuente evaluamos la regla
        {
            it = itReglas->antec.begin();     //Establecemos un puntero al principio de antecedentes de la regla
            if (itReglas->tipo == REGLA_Y || itReglas->tipo == REGLA_O)    //Si la regla contiene la conjunción y u o aplicaremos el caso 1
            {
                if(itReglas->tipo == REGLA_O)
                {
                    operador = *it;    //Guardamos el primer antecedente
                    it++;   //Avanzamos en la lista de antecedentes
                    obj = max(motInferencia(operador),motInferencia(*it));    //Aplicamos el caso 1 para los dos primeros antecedentes
                    it++;   
                    while (it!=itReglas->antec.end())
                    {
                        obj = max(obj,motInferencia(*it));  //Aplicamos el caso 1 para los demás antecedentes, por lo que llamamos recursivamente al motor de inferencia con cada nuevo antecedente
                        it++;
                    } 
                    operador = " or ";     //Guardamos el operador en forma de string
                }
                else
                {
                    operador = *it;    //Guardamos el primer antecedente
                    it++;   //Avanzamos en la lista de antecedentes
                    obj = min(motInferencia(operador),motInferencia(*it));     //Aplicamos el caso 1 para los dos primeros antecedentes
                    it++;
                    while (it!=itReglas->antec.end())
                    {
                        obj = min(obj,motInferencia(*it));  //Aplicamos el caso 1 para los demás antecedentes, por lo que llamamos recursivamente al motor de inferencia con cada nuevo antecedente
                        it++;
                    } 
                    operador = " and ";    //Guardamos el operador en forma de string
                }
                //Mostramos en el fichero salida la combinación de antecedentes que se va a hacer
                it = itReglas->antec.begin();
                ficSalida << "Combinación de antecedentes de " << itReglas->id << " (Caso 1):" << "\nFC(" << *it;

                while (it!=itReglas->antec.end())
                {
                    ficSalida << operador << *it;
                    it++;
                }

                ficSalida << ") = " << obj << endl;
            }
            else
             obj = motInferencia(*it);  //Aplicamos el caso 3 y sabemos que tenemos solo un antecedente en la regla

        fcLista.push_back(itReglas->factorCert * max((double)0,obj)); //Aplicamos el caso 3
        ficSalida << "Combinación de la evidencia con la regla " << itReglas->id << " (Caso 3): \n" << "FC(" << objetivo << "_" << itReglas->id << ") = " << itReglas->factorCert << " * " << "max(0, "<< obj << ") = " <<  fcLista.back() << endl;
        reglas += ", " + itReglas->id;  //Añadimos en el string reglas el id de las reglas que tienen como consecuente el objetivo que estamos evaluando
        }
        itReglas++;
    }

    if(fcLista.size() > 1)  //Si se cumple el if tenemos más de una regla para una hipótesis y aplicaremos el caso 2 en consecuencia
    {
        list<double>::iterator itLista = fcLista.begin();
        double valor;
        obj = *itLista;
        itLista++;
        
        ficSalida << "Combinación de las reglas" << reglas << " (Caso 2):" << endl;

        //Vamos aplicando las reglas del caso 2 con el fin de obtener el factor de certeza final del objetivo
        while (itLista!=fcLista.end())
        {
            valor = *itLista;
            if (valor <= 0 && obj <= 0)
              obj += valor * (1+obj);
            else if(valor >= 0 && obj >= 0)
              obj += valor * (1-obj);
            else
             obj = (obj + valor) / (1-min(fabs(obj),fabs(valor)));
            itLista++;
        }
    }
    else
     obj = fcLista.front();

    ficSalida << "FC(" << objetivo << ") = " << obj << endl;
    actualizarBH(objetivo, obj);  //Actualizamos la base de hechos
    return obj;
}

void liberarMem()   //Función encargada de liberar la memoria eliminando los hechos y las reglas de sus respectivas listas
{
    list<regla>::iterator itc = baseConocimiento.begin();
    while (itc!=baseConocimiento.end())
    {
        while(itc->antec.size()!=0)
        {
            delete &*itc->antec.begin();     //Vamos liberando la memoria del primer elemento de la lista de antecedentes de la regla
            itc->antec.pop_front();
        }
        delete &*itc; //Borramos la regla
        itc++;
    }
    itHechos = baseHechos.begin();
    while (itHechos!=baseHechos.end())
    {
        delete nuevoHecho;
        itHechos++;
    }
}

int main(int argc, char **argv) //Función principal del programa
{
    ifstream BC;     //Variables correspondientes a los ficheros de la bc y la bh
    ifstream BH;
    string nombreBC, nombreBH; //Nombre de la base de conocimiento y base de hechos factorizados

    if(argc!=3)  //Mostramos un error si obtenemos un numero incorrecto de argumentos
        print_error(0);

    BC.open(argv[1],ios::in); //Se abre el fichero BC en modo lectura
    if(BC.fail())  //Mostramos un error si no se puede abrir el fichero
        print_error(1);
    
    BH.open(argv[2],ios::in); //Se abre el fichero BH en modo lectura
    if(BH.fail())  //Mostramos un error si no se puede abrir el fichero
        print_error(2);

    string nConcatenado, arg1, arg2;

   //Extraemos el nombre (sin la extensión) de los ficheros que contienen la BC y la BH

    arg1 = argv[1];   //Obtenemos el nombre de la BC
    arg2 = argv[2];   //Obtenemos el nombre de la BH

    //Factorizamos los nombre de los ficheros
    nombreBC = factorizarNombre(arg1);
    nombreBH = factorizarNombre(arg2);

    //Llamamos a la función que concatena el nombre de ambos ficheros
    nConcatenado = concatenarFicheros(nombreBC, nombreBH);
    
    //Creamos el fichero de salida si no existe
    ficSalida.open(nConcatenado.c_str(),ios::out);

    ficSalida << "Base de conocimientos : " << nombreBC << "\nBase de hechos: " << nombreBH << endl;

    int nReglas, i=0;
    string cadenaBC, cadenaBH;

    // LEEMOS LA BC
    BC >> nReglas;  // Obtenemos el número de reglas
    
    nuevaRegla = new regla; //Creamos una nueva regla a la que asignamos como R1
    nuevaRegla->id = "R1";

    while (i<nReglas)
    {
        BC >> nombreBH;

        if (nombreBH=="Si" || nombreBH=="o" || nombreBH=="y")
        {
            //Establecemos el tipo de la regla
            if(nombreBH=="y")
             nuevaRegla->tipo = REGLA_Y;
            else if(nombreBH=="o")
             nuevaRegla->tipo = REGLA_O;

            BC >> nombreBH;    //Extraemos el antecedente
            nuevaRegla->antec.push_back(nombreBH);  //Añadimos el antecedente en la lista de antecedentes de la regla

        }
        else if (nombreBH=="Entonces")
        {
            BC >> nombreBH;

            nombreBH.erase(nombreBH.length()-1);  //Borramos la coma
            nuevaRegla->consec = nombreBH;    //Establecemos el consecuente a la regla

            BC >> nombreBH;

            cadenaBC = nombreBH;
            cadenaBC.erase(0,3);  //Nos quedamos con el factor de certeza de la regla
            nuevaRegla->factorCert = atof(cadenaBC.c_str());    //Convertimos a double
            baseConocimiento.push_back(*nuevaRegla);  //Añadimos la nueva regla a la base de conocimientos

            i++;
            nuevaRegla = new regla;    //Reservamos memoria para una nueva regla

            BC >> nombreBH;    //Leemos el identificador de la siguiente regla
            nombreBH.erase(nombreBH.length()-1);  //Borramos los dos puntos
            nuevaRegla->id = nombreBH;     //Establecemos el id de la regla
        }
    }

    //LEEMOS LA BH

    BH >> nReglas;   //Obtenemos el numero de hechos iniciales
    string objetivo; //Declaramos un string para el objetivo
    i=0;        //Reiniciamos las variable i
    while (i<nReglas)
    {
        BH >> nombreBH;

        nombreBH.erase(nombreBH.length()-1);  //Borramos la coma

        nuevoHecho = new hecho;   //Creamos un nuevo hecho
        nuevoHecho->h = nombreBH;  //Establecemos el identificador del nuevo hecho

        BH >> nombreBH;    //Leemos el factor de certeza del hecho

        cadenaBH = nombreBH;
        cadenaBH.erase(0,3);  //Nos quedamos con el factor de certeza del hecho
        nuevoHecho->factorCert = atof(cadenaBH.c_str());;   //Convertimos a double
        baseHechos.push_back(*nuevoHecho);  //Añadimos el nuevo hecho a la lista de hechos

        if(i == nReglas-1)  //Si es la última iteración leemos el objetivo
        {
            BH >> nombreBH;
            BH >> objetivo;
        }
        i++;
    }

    // Imprimimos el objetivo junto al comienzo del archivo
    ficSalida << "Objetivo: " << objetivo << endl << endl;
    ficSalida << "Proceso de inferencia:" << endl << endl << motInferencia(objetivo);

    //Vaciamos la memoria reservada borrando la reglas y los hechos
    liberarMem();

    //Cerramos los ficheros abiertos
    BC.close();
    BH.close();
    ficSalida.close();

    return 0;
}