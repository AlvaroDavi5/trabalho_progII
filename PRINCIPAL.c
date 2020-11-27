#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


// definicao de constantes globais
#define TAMVETOR 202362
#define FALSE 0
#define TRUE 1


// definicao de tipos complexos
typedef struct
{
	int ano;
	int mes;
	int dia;
} tData; // formato ano-mes-dia para facilitar armazenamento das datas

typedef struct
{
	tData DataCadastro;
	tData DataObito;
	char Classificacao[15];
	char Municipio[35];
	int IdadeNaDataNotificacao;
	char ComorbidadePulmao[5];
	char ComorbidadeCardio[5];
	char ComorbidadeRenal[5];
	char ComorbidadeDiabetes[5];
	char ComorbidadeTabagismo[5];
	char ComorbidadeObesidade[5];
	char FicouInternado[15];
} tDadosPaciente; // dados de cada linha/paciente

typedef struct
{
	char nomeMun[35];
	int casosConfMun;
} tMunicipiosECasos; // estrutura com nome do municipio e numero de casos [para contabilizar casos por municipios]



// variaveis de escopo global
char matrizMunicipios [78][35] =
{
	"AFONSO CLAUDIO", "AGUA DOCE DO NORTE", "AGUIA BRANCA", "ALEGRE", "ALFREDO CHAVES", "ALTO RIO NOVO", "ANCHIETA", "APIACA",
	"ARACRUZ", "ATILIO VIVACQUA", "BAIXO GUANDU", "BARRA DE SAO FRANCISCO", "BOA ESPERANCA", "BOM JESUS DO NORTE", "BREJETUBA",
	"CACHOEIRO DE ITAPEMIRIM", "CARIACICA", "CASTELO", "COLATINA", "CONCEICAO DA BARRA", "CONCEICAO DO CASTELO", "DIVINO DE SAO LOURENCO",
	"DOMINGOS MARTINS", "DORES DO RIO PRETO", "ECOPORANGA", "FUNDAO", "GOVERNADOR LINDENBERG", "GUACUI", "GUARAPARI", "IBATIBA", "IBIRACU",
	"IBITIRAMA", "ICONHA", "IRUPI", "ITAGUACU", "ITAPEMIRIM", "ITARANA", "IUNA", "JAGUARE", "JERONIMO MONTEIRO", "JOAO NEIVA", "LARANJA DA TERRA",
	"LINHARES", "MANTENOPOLIS", "MARATAIZES", "MARECHAL FLORIANO", "MARILANDIA", "MIMOSO DO SUL", "MONTANHA", "MUCURICI", "MUNIZ FREIRE", "MUQUI",
	"NOVA VENECIA", "PANCAS", "PEDRO CANARIO", "PINHEIROS", "PIUMA", "PONTO BELO", "PRESIDENTE KENNEDY", "RIO BANANAL", "RIO NOVO DO SUL", "SANTA LEOPOLDIN",
	"SANTA MARIA DE JETIBA", "SANTA TERESA", "SAO DOMINGOS DO NORTE", "SAO GABRIEL DA PALHA", "SAO JOSE DO CALCADO", "SAO MATEUS", "SAO ROQUE DO CANAA",
	"SERRA", "SOORETAMA", "VARGEM ALTA", "VENDA NOVA DO IMIGRANTE", "VIANA", "VILA PAVAO", "VILA VALERIO", "VILA VELHA", "VITORIA"
}; // matriz de municipios para comparacao, onde 78 e a quantidade de municipios do ES e 35 o tamanho maximo das strings com os nomes

tMunicipiosECasos vetorMunicipios[78]; // vetor de struct para contabilizar casos de municipios

tDadosPaciente vetorPaciente[TAMVETOR]; // definido vetor e tamanho do vetor, definido como global para evitar falha de segmentacao



// prototipos de funcoes
int contadorDeLinhas(FILE *arq);
void lerEntrada();
void lerArquivoCSV(FILE *arq);
tData filtrarDatas();
void cidadesMaisNCasosOrdemAlfab(int Ncasos); // para item3
int totalDeCasosMun(char muni[]);
void totalCasosEntreD1eD2(tData casosD1, tData casosD2);// para item4
int datasCoincidem(tData data1, tData data2);
tData dataSeguinte(tData data1);
void topNCidades(int topNcasos, tData data1, tData data2); // para item5
int contarCasosEntreD1eD2Muni(tData data1, tData data2, char muni[]);
void ordenarDecresc(tMunicipiosECasos casosMuni[]);
void percentConfInter(char muni[]); //para item6
void percentMortes(char muni[]);
void percentInterMorte(char muni[]);
void Media_DPidades_percentMortesSemComorb_entreD1eD2(tData confMortD1, tData confMortD2); //para item7
float mediaAritm(tData dataNula, int *contIdades);
float desvioPadrao(tData dataNula, int contIdades, float media);

int quantidadeDiasMes(int mes, int ano);
int ehBissexto(int ano);
float calcularPercentual(int num, int total);
void pularPrimeiraLinha(FILE *arq);


// ---------------------------------------------------------------------------------------------


// funcao principal
int main()
{
	FILE *arq; // ponteiro de arquivo, armazena o endereco das posicoes do arquivo

	arq = fopen("./data/covid19ES.csv", "r"); // abrir arquivo (endereco_arquivo, MODO_abertura-leitura), funcao passando por referência

	//int tamVetor = contadorDeLinhas(arq); // definir dinamicamente tamanho do vetor baseado na quantidade de linhas do arquivo [deve ser usada apenas quando se for ler arquivos diferentes]

	if (arq == NULL) // caso o arquivo nao exista, a funcao retorna um ponteiro nulo (NULL)
	{
		printf("Erro na abertura: arquivo nao encontrado!\n");
		exit(1); // forca o encerramento do programa (POR CONVENÇÃO: retorna 0 caso tudo ocorra bem, retorna um número diferente de 0 caso ocorra um erro)
	}

	pularPrimeiraLinha(arq); // ignora os primeiros caracteres ate o \n, ou seja, ate o fim da primeira linha
	lerArquivoCSV(arq);
	lerEntrada();

	fclose(arq); // fechar arquivo e limpar o que foi armazenado no buffer

	return 0;
}


// ---------------------------------------------------------------------------------------------


// todas as funcoes
int contadorDeLinhas(FILE *arq)
{
	int numLinhas = 0; // variavel de contagem

	char c, letra = '\n';

	while(fread (&c, sizeof(char), 1, arq))
	{
		if (feof(arq))
			break;
		if(c == letra)
		{
			numLinhas++;
		}
	}

	rewind(arq); // reinicia o apontador doa posicao de leitura do arquivo

	return numLinhas - 2; // removidas primeira e ultima linhas
}

void lerArquivoCSV(FILE *arq)
{
	int i;

	for (i = 0; i < TAMVETOR; i++) // a estrutura de repeticao preenchera todos os elementos do vetor ate o tamanho maximo
	{
		fscanf(arq, "%d-%d-%d,", &vetorPaciente[i].DataCadastro.ano, &vetorPaciente[i].DataCadastro.mes, &vetorPaciente[i].DataCadastro.dia); // lendo dados do arquivo csv
		fscanf(arq, "%d-%d-%d,", &vetorPaciente[i].DataObito.ano, &vetorPaciente[i].DataObito.mes, &vetorPaciente[i].DataObito.dia);
		fscanf(arq, "%[^,],%[^,],", vetorPaciente[i].Classificacao, vetorPaciente[i].Municipio);
		fscanf(arq, "%*c%d %*[^\"]%*c,", &vetorPaciente[i].IdadeNaDataNotificacao); // usado operador %*c para descartar as informacoes alem da idade em anos
		fscanf(arq, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],", vetorPaciente[i].ComorbidadePulmao, vetorPaciente[i].ComorbidadeCardio, vetorPaciente[i].ComorbidadeRenal, vetorPaciente[i].ComorbidadeDiabetes, vetorPaciente[i].ComorbidadeTabagismo, vetorPaciente[i].ComorbidadeObesidade);
		fscanf(arq, "%[^\n]", vetorPaciente[i].FicouInternado); // o operador %[^,] le os dados como string e para (por isso o ^) ao encontrar ','
	}
}

void lerEntrada()
{
	char dir[40];
	int Ncasos;
	tData casosD1, casosD2;
	int topNcasos;
	tData topNCData1, topNCData2;
	char muni[35];
	tData confMortD1, confMortD2;

	scanf("%s\n", dir); // ler diretorio de salvamento escolhido no input
	/*

	int created = mkdir("PASTA", 0777)

	if (created == 0)
	{
		printf("Deu certo!\n");
	}
	else
	{
		printf("Deu ruim!\n");
	}

	-----------------------------------------

	char comando[40]; 

	strcpy(comando,"mkdir c:\\");
	strcat(comando, dir);

	system(comando);

	-----------------------------------------

	int teste;

	teste = mkdir(dir);
	if (! teste)
		printf("DIRETORIO CRIADO\n");
	else
	{
		printf("DIRETORIO não CRIADO\n");
	}

	-----------------------------------------

	printf("Digite o arquivo que deseja abrir: ");
	gets(arquivo);
	arq = fopen(arquivo, "r");

	*/

	scanf("%d\n", &Ncasos); // ler numero de casos confirmados [para listar em ordem alfabetica as cidades com mais de tais casos]

	casosD1 = filtrarDatas(); // ler intervalo de datas [D1, D2] para contagem de casos confirmados
	casosD2 = filtrarDatas();

	scanf("%d ", &topNcasos); // ler intervalo de datas para listar top N cidades
	topNCData1 = filtrarDatas();
	topNCData2 = filtrarDatas();

	scanf("%s\n", muni);
	int tamStr = strlen(muni); // pegar tamanho da string
	for (int k = 0; k < tamStr; k++)
	{
		muni[k] = toupper(muni[k]); // converter para mauscula cada letra do vetor de caracteres
	}
	confMortD1 = filtrarDatas(); // ler intervalo de datas para calcular percentual de mortes, internacoes e ambos
	confMortD2 = filtrarDatas();

	//executar funcoes dos items
	cidadesMaisNCasosOrdemAlfab(Ncasos);
	totalCasosEntreD1eD2(casosD1, casosD2);
	topNCidades(topNcasos, topNCData1, topNCData2);
	printf("- Resultados para %s:\n", muni);
	percentConfInter(muni);
	percentMortes(muni);
	percentInterMorte(muni);
	Media_DPidades_percentMortesSemComorb_entreD1eD2(confMortD1, confMortD2);
}

tData filtrarDatas()
{
	tData data;

	scanf("%d-%d-%d", &data.ano, &data.mes, &data.dia); // ler ano-mes-dia

	return data;
}

void cidadesMaisNCasosOrdemAlfab(int Ncasos)
{
	int i, todosCasos;

	for (i = 0; i < 78; i++) // verificando cada municipio em ordem alfabetica
	{
		todosCasos = totalDeCasosMun(matrizMunicipios[i]); // verificando se municipio de indice i possui casos confirmados e contando o total

		if (todosCasos > Ncasos) // se total de casos de um dado municipio for maior que o numero minimo de casos...
		{
			printf("- %s: %d casos\n", matrizMunicipios[i], todosCasos); // imprimir municipio e seu total de casos
		}
	}
}

int totalDeCasosMun(char muni[])
{
	int i, total = 0;

	for (i = 0; i < TAMVETOR; i++) // verificar em todo o vetor de struct
	{
		if (strcmp(muni, vetorPaciente[i].Municipio) == 0) // funcao strcmp compara strings e retorna: 0 se as strings forem identicas, valor > 0 se a string1 e maior que a string2 e valor < 0 se strng1 e menos que string2
		{
			if (strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) // comparando para contar apenas casos de covid confirmados (se nao houver diferenca retorna 0)
			{
				total++;
			}
		}
	}

	return total;
}

void totalCasosEntreD1eD2(tData casosD1, tData casosD2)
{
	int casosTotal = 0, i;

	while (! datasCoincidem(casosD1, casosD2)) // varrer de D1 a D2
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if (datasCoincidem(vetorPaciente[i].DataCadastro, casosD1)) // se a data de cadastro coincidir com a D1 que esta sofrendo mudancas no laco... 
			{
				if (strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) // se a classificacao do paciente for confirmada para o covid...
				{
					casosTotal++;
				}
			}
		}
		casosD1 = dataSeguinte(casosD1);
	}

	printf("- Total de pessoas: %d\n", casosTotal);
}

int datasCoincidem(tData data1, tData data2)
{
	if ((data1.dia == data2.dia) && (data1.mes == data2.mes))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

tData dataSeguinte(tData data1)
{
	int qtdDiasD1 = quantidadeDiasMes(data1.mes, data1.ano); // verificando quantidade de dias da D1

	if (data1.mes == 12 && qtdDiasD1 == data1.dia) // iniciando novo ano se a data for o ultimo dia do ano
	{
		data1.dia = 1;
		data1.mes = 1;
		data1.ano++;
	}
	else if (data1.dia == qtdDiasD1) // entrando apenas em um novo mes, caso o dia seja o ultimo do mes
	{
		data1.dia = 1;
		data1.mes++;
	}
	else if (data1.dia < qtdDiasD1) // apenas aumentando o dia ate a quantidade maxima de dias do mes
	{
		data1.dia++;
	}

	return data1;
}

void topNCidades(int topNcasos, tData data1, tData data2)
{
	tMunicipiosECasos casosMuni[78];

	for (int i = 0; i < 78; i++) // verificando em toda a matriz dos municipios
	{
		casosMuni[i].casosConfMun = contarCasosEntreD1eD2Muni(data1, data2, matrizMunicipios[i]); // contando casos confirmados entre D1 e D2 e informar municipio da posicao i
		strcpy(casosMuni[i].nomeMun, matrizMunicipios[i]); // a funcao strcpy copia a string1 para onde se encontra a string2
	}

	ordenarDecresc(casosMuni); // ordenando vetor

	for(int j = 0; j < topNcasos; j++)
	{
		printf("%s: %d\n", casosMuni[j].nomeMun, casosMuni[j].casosConfMun);
	}
}

int contarCasosEntreD1eD2Muni(tData data1, tData data2, char muni[])
{
	data2.dia += 1;
	int qtdCasos = 0, i;

	while (! datasCoincidem(data1, data2)) // varrer de D1 a D2
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if (datasCoincidem(vetorPaciente[i].DataCadastro, data1)) // se a data de cadastro coincidir com a D1 que esta sofrendo mudancas no laco... 
			{
				if (strcmp(vetorPaciente[i].Municipio, muni) == 0) // compara municipio do paciene no vetor ao municipio informado
				{
					if (strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) // se a classificacao do paciente for confirmada para o covid...
					{
						qtdCasos++;
					}
				}
			}
		}
		data1 = dataSeguinte(data1);
	}

	return qtdCasos;
}

void ordenarDecresc(tMunicipiosECasos* casosMuni)
{
	char nomeMunicipio[35]; // variaveis temporarias
	int numCasos;

	// metodo de ordenacao para varrer todo o vetor e comparar quantidade de casos confirmados, ordenando do maior para o menor
	for (int j = 0; j < 77; j++)
	{
		for (int k = 0; k < 76-j; k++)
		{
			if(casosMuni[k].casosConfMun < casosMuni[k+1].casosConfMun)
			{
				numCasos = casosMuni[k].casosConfMun; // anotando casos confirmados
				strcpy(nomeMunicipio, casosMuni[k].nomeMun); // copiando nome do municipio com funcao strcpy
				casosMuni[k].casosConfMun = casosMuni[k+1].casosConfMun;
				strcpy(casosMuni[k].nomeMun, casosMuni[k+1].nomeMun);
				casosMuni[k+1].casosConfMun = numCasos;
				strcpy(casosMuni[k+1].nomeMun, nomeMunicipio);
			}
		}
	}
}

void percentConfInter(char muni[])
{
	int i, internConf = 0, qtdCasosConf = 0;

	if (strcmp(muni, "TODAS") == 0) // caso seja pedido o percentual de todas as cidades
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if ((strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) && (strcmp(vetorPaciente[i].FicouInternado, "Sim") == 0)) // contando todos os casos confirmados com internacao
			{
				internConf++;
			}
			if (strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) // contando todos os casos confirmados apenas
			{
				qtdCasosConf++;
			}
		}
	}
	else // caso seja pedido o percentual de uma cidade em especifico
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if ((strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) && (strcmp(vetorPaciente[i].FicouInternado, "Sim") == 0) && (strcmp(vetorPaciente[i].Municipio, muni) == 0))  // contando os casos confirmados com internacao de um dado municipio
			{
				internConf++;
			}
			if ((strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) && (strcmp(vetorPaciente[i].Municipio, muni) == 0))
			{
				qtdCasosConf++;
			}
		}
	}

	printf("- A %% de pessoas com Covid-19 que ficaram internadas: %.3f%%\n", calcularPercentual(internConf, qtdCasosConf));
}

void percentMortes(char muni[])
{
	tData dataNula;
	int i, mortes = 0, qtdCasosConf = 0;

	dataNula.dia = 0;
	dataNula.mes = 0;
	dataNula.ano = 0;

	if (strcmp(muni, "TODAS") == 0) // caso seja pedido o percentual de todas as cidades
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if (strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) // contando todos os casos confirmados
			{
				qtdCasosConf++;

				if (! datasCoincidem(dataNula, vetorPaciente[i].DataObito)) // se data de obito nao e nula, contar obito
				{
					mortes++;
				}
			}
		}
	}
	else // caso seja pedido o percentual de uma cidade em especifico
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if ((strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) && (strcmp(vetorPaciente[i].Municipio, muni) == 0)) // contando todos os casos confirmados de um dado municipio
			{
				qtdCasosConf++;

				if (! datasCoincidem(dataNula, vetorPaciente[i].DataObito)) // se data de obito nao e nula, contar obito
				{
					mortes++;
				}
			}
		}
	}

	printf("- A %% de pessoas com Covid-19 que morreram: %.3f%%\n", calcularPercentual(mortes, qtdCasosConf));
}

void percentInterMorte(char muni[])
{
	tData dataNula;
	int i, interMortes = 0, qtdMortes = 0;

	dataNula.dia = 0;
	dataNula.mes = 0;
	dataNula.ano = 0;

	if (strcmp(muni, "TODAS") == 0) // caso seja pedido o percentual de todas as cidades
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if ((strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) && (! datasCoincidem(dataNula, vetorPaciente[i].DataObito))) // contando todos os casos confirmados com morte
			{
				qtdMortes++;

				if ((strcmp(vetorPaciente[i].FicouInternado, "Sim") == 0)) // contando todos os casos confirmados com morte e intenacao
				{
					interMortes++;
				}
			}
		}
	}
	else // caso seja pedido o percentual de uma cidade em especifico
	{
		for (i = 0; i < TAMVETOR; i++)
		{
			if ((strcmp(vetorPaciente[i].Classificacao, "Confirmados") == 0) && (! datasCoincidem(dataNula, vetorPaciente[i].DataObito))) // contando todos os casos confirmados com morte de um dado municipio
			{
				qtdMortes++;

				if ((strcmp(vetorPaciente[i].FicouInternado, "Sim") == 0)) // contando todos os casos confirmados com morte e intenacao
				{
					interMortes++;
				}
			}
		}
	}

	printf("- A %% de pessoas que ficaram internadas e morreram: %.3f%%\n", calcularPercentual(interMortes, qtdMortes));
}

void Media_DPidades_percentMortesSemComorb_entreD1eD2(tData confMortD1, tData confMortD2)
{
	int i, contIdades = 0;
	float media, desviopadrao, mortesSemComorb;
	tData dataNula;

	dataNula.dia = 0;
	dataNula.mes = 0;
	dataNula.ano = 0;


	while (! datasCoincidem(confMortD1, confMortD2)) // varrer de D1 a D2
	{
		for (i = 0; i < TAMVETOR; i++) // varrer o vetor de pacientes varias vezes
		{
			if (datasCoincidem(vetorPaciente[i].DataCadastro, confMortD1)) // se a data de cadastro coincidir com a D1 que esta sofrendo mudancas no laco... 
			{
				media = mediaAritm(dataNula, &contIdades);
				desviopadrao = desvioPadrao(dataNula, contIdades, media);
				//MSC
			}
		}
		confMortD1 = dataSeguinte(confMortD1); // aumentar D1 ate coincidir com D2
	}

	printf("A média e desvio padrão da idade: %.3f -- %.3f\n", media, desviopadrao);
	//printf("A %% de pessoas que morreram sem comorbidade: %.3f%%\n", mortesSemComorb);
}

float mediaAritm(tData dataNula, int *contIdades)
{
	int j, somaIdades = 0;
	float media;

	for (j = 0; j < TAMVETOR; j++) // varrer todo o vetor dos pacientes
	{
		if ((! datasCoincidem(dataNula, vetorPaciente[j].DataObito)) && (strcmp(vetorPaciente[j].Classificacao, "Confirmados") == 0)) // contabilizar apenas pessoas com covid que morreram
		{
			somaIdades += vetorPaciente[j].IdadeNaDataNotificacao; // somatorio de idades
			*contIdades++; // contagem de idades dos pacientes mortos que tinham covid
		}
	}
	media = (somaIdades / *contIdades); // media calculada

	return media;
}

float desvioPadrao(tData dataNula, int contIdades, float media)
{
	int j;
	float somaQuadDifIdadeM = 0, desvpadr;

	for (j = 0; j < TAMVETOR; j++) // varrer todo o vetor novamente (dessa vez com a media)
	{
		if ((! datasCoincidem(dataNula, vetorPaciente[j].DataObito)) && (strcmp(vetorPaciente[j].Classificacao, "Confirmados") == 0)) // contabilizar apenas pessoas com covid que morreram
		{
			somaQuadDifIdadeM += ((vetorPaciente[j].IdadeNaDataNotificacao - media)*(vetorPaciente[j].IdadeNaDataNotificacao - media)); // somatorio do quadrado das diferencas entre idades e media
		}
	}
	desvpadr = sqrt((somaQuadDifIdadeM / contIdades)); // desvio padrao calculado

	return desvpadr;
}

float mortesSemComorb()
{
	//
}

int quantidadeDiasMes(int mes, int ano)
{
	int diasMes = 0;

	if (mes == 2)
	{
		if (ehBissexto(ano))
		{
			diasMes = 29;
		}
		else
		{
			diasMes = 28;
		}
	}
	else if (mes == 4 || mes == 6 || mes == 9 || mes == 11)
	{
		diasMes = 30;
	}
	else
	{
		diasMes = 31;
	}

	return diasMes;
}

int ehBissexto(int ano)
{
	return (((ano % 4 == 0) && (ano % 100 != 0)) || (ano % 400 == 0));
}

float calcularPercentual(int num, int total)
{
	return (num * 100) / total;
}

void pularPrimeiraLinha(FILE *arq)
{
	while (fgetc(arq) != '\n')
	{
		// apenas para esquecer a primeira linha
	}
}
