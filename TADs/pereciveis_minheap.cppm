module;
#include <memory>
export module TAD_minheap;
using std::unique_ptr;
using std::make_unique;


export struct minHeap {
    static constexpr int capacidade = 8;
    int tamanho = 0;
    unique_ptr<int[]> pereciveis = make_unique<int[]>(capacidade);
};

void heapify(minHeap &heap, int i){
    int menor = i;
    int esquerda = 2 * i + 1;
    int direita = 2 * i + 2;

    if (esquerda < heap.tamanho && heap.pereciveis[esquerda] < heap.pereciveis[menor])
        menor = esquerda;

    if (direita < heap.tamanho && heap.pereciveis[direita] < heap.pereciveis[menor])
        menor = direita;

    if (menor != i) {
        std::swap(heap.pereciveis[i], heap.pereciveis[menor]);
        heapify(heap, menor);
    }
}

void siftup(minHeap &heap, int i) {
    while (i > 0 && heap.pereciveis[(i - 1) / 2] > heap.pereciveis[i]) {
        std::swap(heap.pereciveis[i], heap.pereciveis[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

export bool removerHeap(minHeap &heap) {
    if (heap.tamanho == 0) {
        return false;
    }
    heap.pereciveis[0] = heap.pereciveis[heap.tamanho - 1];
    heap.tamanho--;
    heapify(heap, 0);
    return true;
}

export bool inserir(minHeap &heap, int valor) {
    if (heap.tamanho == heap.capacidade) {
        return false;
    }
    heap.tamanho++;
    heap.pereciveis[heap.tamanho - 1] = valor;
    siftup(heap, heap.tamanho - 1);
    return true;
}

export void descartarVencidos(minHeap &heap, int turnoAtual) {
    if (heap.tamanho == 0) {
        return;
    }
    while (heap.pereciveis[0] <= turnoAtual) {
        removerHeap(heap);
        if (heap.tamanho == 0) {
            break;
        }
    }
}