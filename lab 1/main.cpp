#include <iostream>
#include <zconf.h>

using namespace std;
 
//Хедер содержит всю информацию о выделяемом блоке
struct header_t
{
    size_t size; //размер выделяемого блока
    unsigned is_free; //пометка, свободен ли блок(0-блок занят, любое число-блок свободен, в него можна записать другой блок памяти)
    struct header_t *next; //так как память представлена в виде связного списка, указатель на следующий елемент
};


struct header_t *head = NULL, *tail = NULL; //голова и хвост, первый и последний елемент всего списка для отображения блоков памяти


// функция поиска свободного блока для malloc
// !используем алгоритм первый походящий!
struct header_t *get_free_block(size_t size)
{
    struct header_t *curr = head; //сначало берем первый елемент списка
    while(curr) {
        if (curr->is_free && curr->size >= size) //если он свободен и размер подходит просто возвращаем этот елемент
            return curr;
        curr = curr->next; // если не подходит, переходим на следующий елемент списка
    }
    return NULL; //если мы прошли список и елемент не найден, просто возвращаем NULL для выделения нового блока в malloc
}

// функция выделения блока памяти, возвращает адресс выделеного блока
void *malloc(size_t size)
{
    size_t total_size; //общий размер вместе с хедером
    void *block; //блок памяти
    struct header_t *header; // хедер

    if (!size) // выходи с функции если размер не указан
        return NULL;


    header = get_free_block(size); // ищим блок указаного размера

    if (header) { //если найден блок, просто помечаем его как не свободный и возвращаем адресс выделеного блока
        header->is_free = 0;
        return (void*)(header + 1);
    }

    total_size = sizeof(struct header_t) + size; //считаем размер выделеного блока с хедером(размер хедера всего 15 байт + размер блока, который мы указали в параметрах)
    block = sbrk(total_size); // sbrk добавляет к завершающему значению(конец кучи) байты и возвращает указатель на старый адрес останова
    if (block == (void*) -1) { //если блок находится за пределами памяти
        return NULL;
    }

    header = static_cast<header_t *>(block); //приводим хедер к типу блока
    header->size = size; // указываем размер блока(без хедера)
    header->is_free = 0; // помечаем блок как не свободный
    header->next = NULL; //следующий елемент - NULL
    if (!head) // если это перый елемент, добавленный в список, он становиться головой
        head = header;
    if (tail) // хвост смещается в конец
        tail->next = header;
    tail = header;

    return (void*)(header + 1); //возвращаем адресс нашего хедера
}


//функция освобождения блока памяти
void free(void *block)
{
    struct header_t *header, *tmp;
    void *programbreak;

    if (!block) //если блок не указан, выходим
        return;



    header = (struct header_t*)block - 1; //получаем указатель на заголовок


    programbreak = sbrk(0); // получаем текущее положение вершины кучи


    if ((char*)block + header->size == programbreak) { // если блок, который мы хоти удалить находиться в конце списка

        if (head == tail) { // если у нас в списке находиться только один елемент, мы можем просто его удалить
            head = tail = NULL;
        }

        // в другом случае удаляем последний елемент списка
        else {
            tmp = head;
            while (tmp) {
                if(tmp->next == tail) {
                    tmp->next = NULL; // удаляем последний елемент списка
                    tail = tmp; // хвост смещаем на предпоследний елемент
                }
                tmp = tmp->next;
            }
        }


        sbrk(0 - sizeof(struct header_t) - header->size); // передав отрицательное число в sbrk мы смещаем конец кучи на размер хедера + размер блока

        return;
    }
    header->is_free = 1; //если нам нужно освободить не конец списка, просто помечем этот блок как свободный

}

void *realloc(void *block, size_t size)
{
    struct header_t *header;
    void *ret;


    if (!block || !size) // если блока не существует, просто возвращаем malloc
        return malloc(size);


    header = (struct header_t*)block - 1; // получаем заголовок блока

    if (header->size >= size) //  смотрим, имеет ли блок уже размер, необходимый для запрошенного размера. Если да, ничего не поделаем.
        return block;
    ret = malloc(size); // Если текущий блок не имеет запрошенного размера, мы вызываем malloc

    if (ret) {

        memcpy(ret, block, header->size); // перемещаем содержимое в новый больший блок с помощью memcpy

        free(block); // освобождаем старый блок памяти
    }

    return ret;
}

void print_mem_list()
{
    header_t *curr = head;
    printf("head = %p, tail = %p \n", (void*)head, (void*)tail);
    while(curr) {
        printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
               (void*)curr, curr->size, curr->is_free, (void*)curr->next);
        curr = curr->next;
    }
}


int main() {


    header_t *header;
    cout<<malloc(1)<<endl;
    cout<<malloc(1)<<endl;
    cout<<malloc(1)<<endl;

    print_mem_list();
    free(head->next);
    print_mem_list();
    malloc(2);
    print_mem_list();


    print_mem_list();

    malloc(1);

    print_mem_list();






    return 0;
}