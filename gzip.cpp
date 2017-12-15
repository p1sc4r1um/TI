/*Author: Rui Pedro Paiva
Teoria da Informa��o, LEI, 2007/2008*/

#include <cstdlib>
#include <iostream>
#include <bitset>
#include "gzip.h"
#include "huffman.h"

//fun��o principal, a qual gere todo o processo de descompacta��o
int main(int argc, char** argv)
{
	//--- Gzip file management variables
	FILE *gzFile;  //ponteiro para o ficheiro a abrir
	long fileSize;
	long origFileSize;
	int numBlocks = 0, i;
	gzipHeader gzh;
	unsigned char byte;  //variável temporária para armazenar um byte lido directamente do ficheiro
	unsigned int rb = 0;  //último byte lido (poderá ter mais que 8 bits, se tiverem sobrado alguns de leituras anteriores)
	char needBits = 0, availBits = 0;
	char hlit, hdist, hclen;



	//--- obter ficheiro a descompactar
	//char fileName[] = "FAQ.txt.gz";

	if (argc != 2)
	{
		printf("Linha de comando inválida!!!");
		return -1;
	}
	char * fileName = argv[1];

	//--- processar ficheiro
	gzFile = fopen(fileName, "r");
	fseek(gzFile, 0L, SEEK_END);
	fileSize = ftell(gzFile);
	fseek(gzFile, 0L, SEEK_SET);

	//ler tamanho do ficheiro original (acrescentar: e definir Vector com s�mbolos
	origFileSize = getOrigFileSize(gzFile);


	//--- ler cabe�alho
	int erro = getHeader(gzFile, &gzh);
	if (erro != 0)
	{
		printf ("Formato inválido!!!");
		return -1;
	}

	//--- Para todos os blocos encontrados
	char BFINAL;
	char* string = (char *)malloc(origFileSize);

	do
	{
		//--- ler o block header: primeiro byte depois do cabeçalho do ficheiro
		needBits = 3;
		if (availBits < needBits)
		{
			fread(&byte, 1, 1, gzFile);
			rb = (byte << availBits) | rb; //byte * 2 ^availBits ou rb
			availBits += 8;
		}
		//obter BFINAL
		//ver se é o último bloco
		BFINAL = rb & 0x01; //primeiro bit é o menos significativo
		printf("BFINAL = %d\n", BFINAL);
		rb = rb >> 1; //descartar o bit correspondente ao BFINAL
		availBits -=1;

		//analisar block header e ver se é huffman dinâmico
		if (!isDynamicHuffman(rb))  //ignorar bloco se não for Huffman dinâmico
			continue;
		rb = rb >> 2; //descartar os 2 bits correspondentes ao BTYPE
		availBits -= 2;

		//--- Se chegou aqui --> compactado com Huffman din�mico --> descompactar
		//**************************************************
		//****** ADICIONAR PROGRAMA... *********************
		//**************************************************
		//1º ex
		read_block(&rb, &availBits, gzFile, &hlit, &hdist, &hclen);
		//2º ex - obter comprimentos dos codigos de huffman
		unsigned int code_len[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		int max;
		max = store_array(&rb, &availBits, hclen, code_len, gzFile);
		printf("\nComprimentos:\n");
		for(i = 0; i< 19; i++) {
			printf("%d\n", code_len[i]);
		}
		printf("\n");
		//3º ex - cria arvore de huffman obtendo codigos atraves dos comprimentos
		unsigned int codes[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		HuffmanTree *hft = createHFTree();
		printf("1ª árvore:\n");
		huffman_codes(19, code_len, codes, max, hft);
		//4º ex
		unsigned int hlit_lens[hlit+257 + (286-(hlit+257))];
		memset(hlit_lens, 0, (hlit+257 + (286-(hlit+257)))*sizeof(unsigned int));
		get_lengths(hft, &rb, &availBits, hlit+257, gzFile, hlit_lens);
		printf("lengths HLIT: \n");
		for(i = 0; i< (hlit+257 + (286-(hlit+257))); i++) {
			printf("\n[%d] - [%d]", i, hlit_lens[i]);
		}
		//5º ex
		unsigned int hdist_lens[hdist+1 + (30-(hdist+1))];
		memset(hdist_lens, 0, (hdist+1 + (30-(hdist+1)))*sizeof(unsigned int));
		get_lengths(hft, &rb, &availBits, hdist+1, gzFile, hdist_lens);
		printf("\nlengths HDIST: \n");
		for(i = 0; i< (hdist+1 + (30-(hdist+1))); i++) {
			printf("\n[%d] - [%d]", i, hdist_lens[i]);
		}
		//6º ex
		printf("\nHLEN:\n");
		unsigned int HLIT_codes[hlit+267];
		memset(HLIT_codes, 0, (hlit+267)*sizeof(unsigned int));
		HuffmanTree *hft_HLIT = createHFTree();
		printf("\n2ª árvore:\n");
		huffman_codes(hlit+257 + (286-(hlit+257)), hlit_lens, HLIT_codes, max_in_array(hlit+267, hlit_lens), hft_HLIT);
		printf("\nHDIST:\n");
		unsigned int HDIST_codes[30];
		memset(HDIST_codes, 0, 30*sizeof(unsigned int));
		HuffmanTree *hft_HDIST = createHFTree();
		printf("\n3ª árvore:\n");
		huffman_codes((hdist+1 + (30-(hdist+1))), hdist_lens, HDIST_codes, max_in_array(30, hdist_lens), hft_HDIST);


		//7º ex
		lz77(hft_HLIT, hft_HDIST, &rb, &availBits, string, gzFile);
		numBlocks+= 1;
	}while(BFINAL == 0);


	//terminações
	fclose(gzFile);
	printf("End: %d bloco(s) analisado(s).\n", numBlocks);
	FILE* f = fopen(gzh.fName, "w");
	fprintf(f,"%s", string);
	fclose(f);
    //teste da função bits2String: RETIRAR antes de criar o executável final
	char str[9];
	bits2String(str, 0x03);
	printf("%s\n", str);


    //RETIRAR antes de criar o execut�vel final
    system("PAUSE");
    return EXIT_SUCCESS;
}
//---------------------------------------------------------------
//L� o cabe�alho do ficheiro gzip: devolve erro (-1) se o formato for inv�lidodevolve, ou 0 se ok


int getHeader(FILE *gzFile, gzipHeader *gzh) //obt�m cabe�alho
{
	unsigned char byte;

	//Identica��o 1 e 2: valores fixos
	fread(&byte, 1, 1, gzFile);
	(*gzh).ID1 = byte;
	if ((*gzh).ID1 != 0x1f) return -1; //erro no cabe�alho

	fread(&byte, 1, 1, gzFile);
	(*gzh).ID2 = byte;
	if ((*gzh).ID2 != 0x8b) return -1; //erro no cabe�alho

	//M�todo de compress�o (deve ser 8 para denotar o deflate)
	fread(&byte, 1, 1, gzFile);
	(*gzh).CM = byte;
	if ((*gzh).CM != 0x08) return -1; //erro no cabe�alho

	//Flags
	fread(&byte, 1, 1, gzFile);
	unsigned char FLG = byte;

	//MTIME
	char lenMTIME = 4;
	fread(&byte, 1, 1, gzFile);
	(*gzh).MTIME = byte;
	for (int i = 1; i <= lenMTIME - 1; i++)
	{
		fread(&byte, 1, 1, gzFile);
		(*gzh).MTIME = (byte << 8) + (*gzh).MTIME;
	}

	//XFL (not processed...)
	fread(&byte, 1, 1, gzFile);
	(*gzh).XFL = byte;

	//OS (not processed...)
	fread(&byte, 1, 1, gzFile);
	(*gzh).OS = byte;

	//--- Check Flags
	(*gzh).FLG_FTEXT = (char)(FLG & 0x01);
	(*gzh).FLG_FHCRC = (char)((FLG & 0x02) >> 1);
	(*gzh).FLG_FEXTRA = (char)((FLG & 0x04) >> 2);
	(*gzh).FLG_FNAME = (char)((FLG & 0x08) >> 3);
	(*gzh).FLG_FCOMMENT = (char)((FLG & 0x10) >> 4);

	//FLG_EXTRA
	if ((*gzh).FLG_FEXTRA == 1)
	{
		//ler 2 bytes XLEN + XLEN bytes de extra field
		//1� byte: LSB, 2�: MSB
		char lenXLEN = 2;

		fread(&byte, 1, 1, gzFile);
		(*gzh).xlen = byte;
		fread(&byte, 1, 1, gzFile);
		(*gzh).xlen = (byte << 8) + (*gzh).xlen;

		(*gzh).extraField = new unsigned char[(*gzh).xlen];

		//ler extra field (deixado como est�, i.e., n�o processado...)
		for (int i = 0; i <= (*gzh).xlen - 1; i++)
		{
			fread(&byte, 1, 1, gzFile);
			(*gzh).extraField[i] = byte;
		}
	}
	else
	{
		(*gzh).xlen = 0;
		(*gzh).extraField = 0;
	}

	//FLG_FNAME: ler nome original
	if ((*gzh).FLG_FNAME == 1)
	{
		(*gzh).fName = new char[1024];
		unsigned int i = 0;
		do
		{
			fread(&byte, 1, 1, gzFile);
			if (i <= 1023)  //guarda no m�ximo 1024 string no array
				(*gzh).fName[i] = byte;
			i++;
		}while(byte != 0);
		if (i > 1023)
			(*gzh).fName[1023] = 0;  //apesar de nome incompleto, garantir que o array termina em 0
	}
	else
		(*gzh).fName = 0;

	//FLG_FCOMMENT: ler coment�rio
	if ((*gzh).FLG_FCOMMENT == 1)
	{
		(*gzh).fComment = new char[1024];
		unsigned int i = 0;
		do
		{
			fread(&byte, 1, 1, gzFile);
			if (i <= 1023)  //guarda no m�ximo 1024 string no array
				(*gzh).fComment[i] = byte;
			i++;
		}while(byte != 0);
		if (i > 1023)
			(*gzh).fComment[1023] = 0;  //apesar de coment�rio incompleto, garantir que o array termina em 0
	}
	else
		(*gzh).fComment = 0;


	//FLG_FHCRC (not processed...)
	if ((*gzh).FLG_FHCRC == 1)
	{
		(*gzh).HCRC = new unsigned char[2];
		fread(&byte, 1, 1, gzFile);
		(*gzh).HCRC[0] = byte;
		fread(&byte, 1, 1, gzFile);
		(*gzh).HCRC[1] = byte;
	}
	else
		(*gzh).HCRC = 0;

	return 0;
}
//---------------------------------------------------------------
//Analisa block header e v� se � huffman din�mico
int isDynamicHuffman(unsigned char rb) {
	unsigned char BTYPE = rb & 0x03; // 0000 0011

	if (BTYPE == 0) //--> sem compressão
	{
		printf("Ignorando bloco: sem compactação!!!\n");
		return 0;
	}
	else if (BTYPE == 1)
	{
		printf("Ignorando bloco: compactado com Huffman fixo!!!\n");
		return 0;
	}
	else if (BTYPE == 3)
	{
		printf("Ignorando bloco: BTYPE = reservado!!!\n");
		return 0;
	}
	else //so vai returnar um se 0000 0010
		return 1;
}
//---------------------------------------------------------------
//Obtém tamanho do ficheiro original
long getOrigFileSize(FILE * gzFile) {
	//salvaguarda posi��o actual do ficheiro
	long fp = ftell(gzFile);

	//�ltimos 4 bytes = ISIZE;
	fseek(gzFile, -4, SEEK_END);

	//determina ISIZE (s� correcto se cabe em 32 bits)
	unsigned long sz = 0;
	unsigned char byte;
	fread(&byte, 1, 1, gzFile);
	sz = byte;
	for (int i = 0; i <= 2; i++)
	{
		fread(&byte, 1, 1, gzFile);
		sz = (byte << 8*(i+1)) + sz;
	}


	//restaura file pointer
	fseek(gzFile, fp, SEEK_SET);

	return sz;
}
//---------------------------------------------------------------
void bits2String(char *strBits, unsigned char byte) {
	char mask = 0x01;  //get LSbit

	strBits[8] = 0;
	for (char bit, i = 7; i >= 0; i--)
	{
		bit = byte & mask;
		strBits[i] = bit +48; //converter valor numérico para o caracter alfanumérico correspondente
		byte = byte >> 1;
	}
}
//------------------------------------------------
void read_block(unsigned int *rb, char *availBits, FILE *gzFile, char *hlit, char *hdist, char *hclen) {
	//Leitura do HLIT
	char needBits = 5; // hlit sao 5 bits
	get_byte_from_block(rb, availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
	*hlit = *rb & 0x1F;								//faz mascara de 1's com o rb para descobrir o hlit 00000011111
	*rb = *rb >> 5;									//Descarta o HLIT acabado de ler
	(*availBits) -= 5;								//Reduz o número de bits disponiveis para analizar
	//Leitura do HDIST
	needBits = 5; //hdist sao 5 bits
	get_byte_from_block(rb, availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
	*hdist = *rb & 0x1F;	//5bits
	*rb = *rb >> 5;
	(*availBits) -= 5;


	//Leitura do HCLEN
	//needBits = 4; //hclen sao 4 bits
	get_byte_from_block(rb, availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
	*hclen = *rb & 0x0F; //4 bits 0000 1111
	*rb = *rb >> 4;
	(*availBits) -= 4;
}

int store_array(unsigned int *rb, char *availBits, char hclen, unsigned int *code_len, FILE *gzFile) {
    int positions[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    unsigned char byte;
    int i;
    int maxbits = 0;
    int needBits = 3;
    for(i = 0; i < hclen + 4; i++) { //hclen vai de 0 a 15 (+4 = tamanho do positions)
        get_byte_from_block(rb, availBits, needBits, gzFile);
        code_len[positions[i]] = *rb & 0x07; //ultimos 3 bits 0000 0111
        *rb = *rb >> 3;
        (*availBits) -= 3;
        if(code_len[positions[i]] > maxbits) { //maxbits -- comprimento maximo do codigo de huffman
            maxbits = code_len[positions[i]];
        }
    }
    return maxbits;
}

int max_in_array(int len, unsigned int array[]) {
	int x = array[0];
	for (int i = 0; i< len; i++) {
		if(array[i] > x) {
			x = array[i];
		}
	}
	return x;
}

char* toBinary(int n, int length) {
	int i;
  std::string str;
  while(n!=0) {str=(n%2==0 ?"0":"1")+str; n/=2;}
	char * writable = new char[length + 1];
	for(i =0; i<str.size(); i++) {
		writable[length-str.size()+i] = str.at(i);
	}
	for(i=0; i<length-str.size(); i++) {
		writable[i] = '0';
	}
	writable[length] = '\0';

  return writable;
}

void huffman_codes (int len, unsigned int *code_lengths, unsigned int *codes, int max, HuffmanTree* hft) {
  int i, j = 0, code = 0;
  for(i=1; i<=max; i++) {
	  for(j = 0; j<len; j++) {
	  	if(code_lengths[j] == i) {
				codes[j] = code;
				code +=1;
	  	}
	  }
		code--;
  	code = (code+1) * 2;
  }
	for(i = 0; i < len; i++) {
		if(code_lengths[i] != 0){
			addNode(hft, toBinary(codes[i], code_lengths[i]), i, 1);
		}
	}
}
void get_lengths(HuffmanTree* ht, unsigned int *rb, char *availBits, int size, FILE *gzFile, unsigned int* codes) {
	memset(codes, 0, size*sizeof(unsigned int));
	int i_node, i = 0, j = 0, extraBits;
	unsigned int bit;
	while(i<size){
		while(1){//vai buscar bit ate chegar a uma folha
			get_byte_from_block(rb, availBits, 1, gzFile);
			bit = *rb&0x1;
			*rb = *rb >> 1;
			(*availBits) -= 1;
			i_node = nextNode(ht, '0' + bit);
			if(i_node >= -1){
				int bits = 0;
				if(i_node == -1){
					codes[i] = 0;
					i++;
				}
				else{
					if(i_node < 16){
						codes[i] = i_node;
						i++;
					}
					else if(i_node == 16){ //se == 16 ver 2 bits a frente para saber quantas posiçoes sao iguais a anterior (3 a 6)
						get_byte_from_block(rb, availBits, 2, gzFile);
						extraBits = (*rb & 0x3) + 3;
						*rb = *rb >> 2;
						(*availBits) -= 2;
						for(j=i; j < i + extraBits; j++){
							codes[j] = codes[i-1];
						}
						i+=extraBits;

					}
					else if(i_node == 17){ //se == 17 ver 3 bits a frente para saber quantas posiçoes sao 0 (3 a 10)
						get_byte_from_block(rb, availBits, 3, gzFile);
						extraBits = (*rb&0x7) + 3;
						*rb = *rb >> 3;
						(*availBits) -= 3;
						for(j=i; j < i + extraBits; j++){
						    codes[j] = 0;
						}
						i+=extraBits;
					}
					else if(i_node == 18){ //se ==18 ver 7 bits a frente para saber quantas posiçoes sao 0 (11 a 130)
						get_byte_from_block(rb, availBits, 7, gzFile);
						extraBits = (*rb&0x7F) + 11;
						*rb = *rb >> 7;
						(*availBits) -= 7;
						for(j=i; j < i + extraBits; j++){
							codes[j] = 0;
						}
						i+=extraBits;
					}
				}
			resetCurNode (ht);
			break;
		  }
		}
	}
}

void lz77(HuffmanTree *ht_LIT, HuffmanTree *ht_DIST, unsigned int *rb, char* availBits, char *string, FILE *gzFile) {
	int extra_length[] = {
	11, 13, 15, 17, 19, 23, 27,
	31, 35, 43, 51, 59, 67, 83,
	99, 115, 131, 163, 195, 227
	};
	int extra_dist[] = {
	5, 7, 9, 13, 17, 25, 33, 49, 65,
	97, 129, 193, 257, 385, 513, 769,
	1025, 1537, 2049, 3073, 4097,
	6145, 8193, 12289, 16385, 24577
	};
	unsigned int mask;
	int i, pos, bits, index = 0;
	while(1){
		get_byte_from_block(rb, availBits, 1, gzFile);
		bits=*rb & 0x01;
		*rb = *rb>>1;
		(*availBits)--;
		pos=nextNode(ht_LIT,'0'+bits);
		if (pos >=-1){
			if (pos<256){
				string[index++]=pos;
			}
			else if(pos==256){
				break;
			}
			else{
				int length;
				if (pos<265){
						length = pos - 254;
				}
				else if(pos<285){
					get_byte_from_block(rb, availBits, (pos-261)/4, gzFile);
					mask = 0;
					for(int i=0; i<((pos-261)/4); i++){
			        mask = mask << 1;
			        mask = mask | 0x01;
			    }
					bits=*rb & mask;
					*rb = *rb>>((pos-261)/4);
					(*availBits) -= (pos-261)/4;
					length = bits + extra_length[pos - 265];
				}
				else{
					length=258;
				}
				int dist;
				while(1){
					get_byte_from_block(rb,availBits,1,gzFile);
					bits=*rb & 0x1;
					*rb = *rb >> 1;
					(*availBits) --;
					pos=nextNode(ht_DIST,'0'+bits);
					if (pos>=0){
						dist=pos+1;
						if (pos>3){
							get_byte_from_block(rb, availBits, (pos-2)/2, gzFile);
							mask = 0;
							for(int i=0; i<((pos-2)/2); i++){
					        mask = mask << 1;
					        mask = mask | 0x01;
					    }
							bits=*rb & mask;
							*rb = *rb >> ((pos-2)/2);
							(*availBits) -= ((pos-2)/2);
							dist = bits + extra_dist[pos - 4] + 1;
						}
						for(i=0; i<length; i++){
							string[index+i] = string[index-dist+i+1];
						}
						index += length;
						break;
					}
				}
			}

		resetCurNode(ht_LIT);
		resetCurNode(ht_DIST);
		}
	}
}

//get 1 more byte if neccessary
void get_byte_from_block(unsigned int *rb, char *availBits, char needBits, FILE *gzFile) {
	unsigned char byte;
	while ((*availBits) < needBits)
	{
		fread(&byte, 1, 1, gzFile);
		*rb = (byte << (*availBits)) | *rb; //shift para acrescentar (n availBits) zeros e faz ou com os bits anteriores
		(*availBits) += 8;
	}
}
