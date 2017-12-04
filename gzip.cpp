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
		//2º ex
		int code_len[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		int max;
		max = store_array(&rb, &availBits, hclen, code_len, gzFile);
		//3º ex
		int codes[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		HuffmanTree *hft = createHFTree();
		huffman_codes(code_len, codes, max, hft);
		//4º ex
		unsigned int hlit_codes[hlit+257];
		get_lengths(hft, &rb, &availBits, hlit+257, gzFile, hlit_codes);
		//5º ex
		unsigned int hdist_codes[hdist+1];
		get_lengths(hft, &rb, &availBits, hdist+1, gzFile, hdist_codes);
		//6º ex
		unsigned int codesHLIT[hlit+257]
		
		
		numBlocks++;
	}while(BFINAL == 0);


	//terminações
	fclose(gzFile);
	printf("End: %d bloco(s) analisado(s).\n", numBlocks);


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
			if (i <= 1023)  //guarda no m�ximo 1024 caracteres no array
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
			if (i <= 1023)  //guarda no m�ximo 1024 caracteres no array
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

int store_array(unsigned int *rb, char *availBits, char hclen, int *code_len, FILE *gzFile) {
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

char* toBinary(int n, int length) {
	int i;
    std::string str;
    while(n!=0) {str=(n%2==0 ?"0":"1")+str; n/=2;}
		char * writable = new char[length + 1];
		//char * writable = new char[str.size() + 1];
		for(i =0; i<str.size(); i++) {
			writable[length-str.size()+i] = str.at(i);
		}
		//std::copy(str.begin(), str.end(), writable);
		for(i=0; i<length-str.size(); i++) {
			writable[i] = '0';
		}
		writable[length] = '\0'; // don't forget the terminating 0

		// don't forget to free the string after finished using it
    return writable;
}

void huffman_codes (int *code_lengths, int *codes, int max, HuffmanTree* hft) {
  int i, j = 0, code = 0;
	char string[10];
  int verifica = 0;
  for(i=1; i<=max; i++) {
	  for(j = 0; j<19; j++) {
	  	if(code_lengths[j] == i) {
				verifica = 1;
				codes[j] = code;
				code +=1;
	  	}
	  }
	  if (verifica==1) {
			code--;
	  	code = (code+1) * 2;
			verifica = 0;
	  }
  }
	for(i = 0; i < 19;i++) {
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
		while(1){
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
					else if(i_node == 16){
						get_byte_from_block(rb, availBits, 2, gzFile);
						extraBits = (*rb & 0x3) + 3;
						*rb = *rb >> 2;
						(*availBits) -= 2;
						for(j=i; j < i + extraBits; j++){
							codes[j] = codes[i-1];
						}
						i+=extraBits;
						
					}
					else if(i_node == 17){	
						get_byte_from_block(rb, availBits, 3, gzFile);
						extraBits = (*rb&0x7) + 3;
						*rb = *rb >> 3;
						(*availBits) -= 3;
						for(j=i; j < i + extraBits; j++){
						    codes[j] = 0;
						}
						i+=extraBits;
					}
					else if(i_node == 18){
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
	for(i = 0; i<size; i++) {
		printf("[%d] - %d\n", i, codes[i]);
	}
}
