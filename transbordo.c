#include <nSystem.h>
#include "transbordo.h"

nMonitor m; 
int* enPargua;
int barcosEnPargua;
int proxConsumidoPargua;
int proxProducidoPargua;

int* enChacao;
int barcosEnChacao;
int proxConsumidoChacao;
int proxProducidoChacao;
int p;

int barcosEnCaminoAChacao;
int barcosEnCaminoAPargua;


void inicializar(int P){
	m = nMakeMonitor();
	p = P;
	// Inicializar las cosas de pargua
	enPargua = nMalloc(sizeof(int) * p);
	for(int i = 0; i < p; i++){
		enPargua[i] = i;
	}
	barcosEnPargua = p;
	proxConsumidoPargua = 0;
	proxProducidoPargua = 0;
	// Inicializar las cosas de chacao
	enChacao = nMalloc(sizeof(int) * p);
	barcosEnChacao = 0;
	proxProducidoChacao = 0;
	proxProducidoChacao = 0;
	barcosEnCaminoAPargua = 0;
	barcosEnCaminoAChacao = 0;
}

void finalizar(){
	nDestroyMonitor(m);
	nFree(enPargua);
	nFree(enChacao);
}

int consumirEnChacao(){
	int barco;
	nEnter(m);
	while(barcosEnChacao==0){
		nWait(m);
	}
	barco = enChacao[proxConsumidoChacao];
	proxConsumidoChacao = (proxConsumidoChacao + 1) % p;
	barcosEnChacao--;
	barcosEnCaminoAPargua++;
	nNotifyAll(m);
	nExit(m);
	return barco;
}

int consumirEnPargua(){
	int barco;
	nEnter(m);
	while(barcosEnPargua==0){
		nWait(m);
	}
	barco = enPargua[proxConsumidoPargua];
	proxConsumidoPargua = (proxConsumidoPargua + 1) % p;
	barcosEnPargua--;
	barcosEnCaminoAChacao++;
	nNotifyAll(m);
	nExit(m);
	return barco;
}

void producirEnChacao(int x){
	nEnter(m);
	while(barcosEnChacao == p){
		nWait(m);
	}
	enChacao[proxProducidoChacao] = x;
	proxProducidoChacao = (proxProducidoChacao + 1) % p;
	barcosEnChacao++;
	barcosEnCaminoAChacao--;
	nNotifyAll(m);
	nExit(m);
}

void producirEnPargua(int x){
	nEnter(m);
	while(barcosEnPargua == p){
		nWait(m);
	}
	enPargua[proxProducidoPargua] = x;
	proxProducidoPargua = (proxProducidoPargua + 1) % p;
	barcosEnPargua++;
	barcosEnCaminoAPargua--;
	nNotifyAll(m);
	nExit(m);
}

void transbordoAChacao(int v){
	nEnter(m);
	while(barcosEnChacao + barcosEnCaminoAChacao == p){
		nExit(m);
		transbordoAPargua(-1);
		nEnter(m);
	}
	nExit(m);
	int x = consumirEnPargua();
	haciaChacao(x,v);
	producirEnChacao(x);
}

void transbordoAPargua(int v){
	nEnter(m);
	while(barcosEnPargua + barcosEnCaminoAPargua == p){
		nExit(m);
		transbordoAChacao(-1);
		nEnter(m);
	} 
	nExit(m);
	int x = consumirEnChacao();
	haciaPargua(x,v);
	producirEnPargua(x);
}