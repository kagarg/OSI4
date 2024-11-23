#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock.h>
#include <locale.h>
#pragma comment(lib, "Ws2_32.lib")

void MAC_print(FILE* out, char* MAC)
{
    for (int i = 0; i < 5; i++)
        fprintf(out, "%02X:", (unsigned char)MAC[i]);
    fprintf(out, "%02X\n", (unsigned char)MAC[5]);
}

void IP_print(FILE* out, char* IP)
{
    for (int i = 0; i < 3; i++)
        fprintf(out, "%d.", (unsigned char)IP[i]);
    fprintf(out, "%d\n", (unsigned char)IP[3]);
}

int main()
{
    setlocale(LC_ALL, "russian");
    FILE* input = NULL;
    char file_name[20]; //переменная для имени файла
    int size = 0; //размер кадра(для каждого кадра)
    //переменные для итогового резульатата обработки
    int count_frame = 0; //переменная для подсчёта количества кадров
    int DIX_fr = 0; //число кадров типа DIX
    int RAW_fr = 0; //число кадров типа RAW
    int SNAP_fr = 0; //число кадров типа SNAP
    int LLC_fr = 0; //число кадров типа LLC
    char* DATA;

    printf("Введите имя файла формата ethersXX.bin, где XX - номер от 01 до 12: ");
    scanf("%s", file_name);
    input = fopen(file_name, "rb");
    if (input == NULL) 
    {
        printf("Ошибка открытия файла. Проверьте правильность ввода имени файла.\n");
        system("pause");
        return 0;
    }

    fseek(input, 0, SEEK_END); 
    size = ftell(input);
    fseek(input, 0, SEEK_SET); 

    DATA = (char*)malloc(size); //выделяем память для массива размером size(в байтах), malloc возвращает указатель на выделенную область памяти и приводит его к типу char*
    fread(DATA, 1, size, input); //считывает size элементов данных, каждый из которых занимает 1 байт, из файла и сохраняет их в массиве DATA.
    

    FILE* res = fopen("output.txt", "w");

    fprintf(res, "Размер файла: %d байтов\n\n", size);

    char* p = DATA;
    while (p < DATA + size)
    {
        count_frame++;
        fprintf(res, "Номер кадра: %d\n", count_frame);
        fprintf(res, "MAC-адрес получателя: ");
        MAC_print(res, p);
        fprintf(res, "MAC-адрес отправителя: ");
        MAC_print(res, p + 6);

        unsigned short LT = ntohs(*(unsigned short*)(p + 12)); //осуществляет перевод целого короткого числа из сетевого порядка байт в порядок байт, принятый на компьютере
        if (LT == 0x0800)
        {
            fprintf(res, "Тип кадра: DIX\n");
            fprintf(res, "IP-источник: ");
            IP_print(res, p + 26);
            fprintf(res, "IP-адрес: ");
            IP_print(res, p + 30);
            LT = ntohs(*(unsigned short*)(p + 16)) + 14;
            fprintf(res, "Размер кадра: %d\n\n", LT);
            DIX_fr++;
            p += LT;
        }
        else
        {
            if (LT == 0x0806)
            {
                fprintf(res, "Тип кадра: DIX\n");
                p += 28 + 14; // длина стандартного ARP-пакета является фиксированной и равна 28 байтам, 14 байтов занимают параметры кадра
                DIX_fr++;
                fprintf(res, "Размер кадра: %d\n\n", 28 + 14);
            }
            else {
                if (LT > 0x05DC)
                {
                    fprintf(res, "Тип кадра: Ethernet DIX (Ethernet II)\n");
                    DIX_fr++;
                }
                else
                {
                    unsigned short F = ntohs(*(unsigned short*)(p + 14));
                    if (F == 0xFFFF)
                    {
                        fprintf(res, "Тип кадра: Raw 802.3 (Novell 802.3)\n");
                        RAW_fr++;
                    }
                    else if (F == 0xAAAA)
                    {
                        fprintf(res, "Тип кадра: Ethernet SNAP\n");
                        SNAP_fr++;
                    }
                    else
                    {
                        fprintf(res, "Тип кадра: 802.3/LLC (Novell 802.2)\n");
                        LLC_fr++;
                    }
                }
                p += LT + 14;
                fprintf(res, "Размер кадра: %d\n\n", LT + 14);
            }
        }
    }
    fprintf(res, "Общее число кадров: %d\n", count_frame);
    fprintf(res, "DIX: %d\n", DIX_fr);
    fprintf(res, "RAW: %d\n", RAW_fr);
    fprintf(res, "SNAP: %d\n", SNAP_fr);
    fprintf(res, "LLC: %d\n", LLC_fr);
    
    printf("Программа успешно выполнена. Результат работы программы сохранён в файле \"output.txt\".\n");

    fclose(input);
    fclose(res);
    system("pause");
    return 0;
}
