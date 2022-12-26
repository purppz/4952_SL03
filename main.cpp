#include <opencv4/opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <unistd.h>

using namespace cv;
using namespace std;
using json = nlohmann::json;

// Turma
// Precisamos da grelha
// Precisamos dos dias da semana
// precisamos das horas do dia

// Para converter pdf para jpg.
// primeiro criar o directorio turma/
// convert -density 200 horario.pdf -quality 100 horario/turma.jpg

struct Coordenadas
{
    int x;
    int inicio;
    int fim;
};

string lerTexto(Mat &img, string lang = "eng")
{

    string text;
    // Create Tesseract object
    tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
    // Initialize tesseract to use English (eng) and the LSTM OCR engine.

    ocr->Init(NULL, lang.c_str(), tesseract::OEM_LSTM_ONLY);

    // Set Page segmentation mode to PSM_AUTO (3)

    ocr->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);

    ocr->SetImage(img.data, img.cols, img.rows, 1, img.step);

    // para remover o warning chato do invalid resolution

    ocr->SetSourceResolution(400);

    // Run Tesseract OCR on image

    text = string(ocr->GetUTF8Text());

    replace(text.begin(), text.end(), '\n', ' ');

    // cout << text << endl;

    // Destroy used object and release memory

    ocr->End();

    return text;
}

void parseTurma(Mat &img)

{

    Mat turmaImg;

    turmaImg = img(Range(285, 340), Range(90, 1485));

    threshold(turmaImg, turmaImg, 0, 255, THRESH_BINARY);

    string texto = lerTexto(turmaImg, "por");

    imshow("turma", turmaImg);

    cout << texto << endl;
}

void parseDaGrelha(Mat &img, Mat &grelha)

{

    // y1, y2, x1, x2

    grelha = img(Range(345, 1770), Range(108, 1460));
}

vector<string> parseHorasOuDiaSemana(Mat &grelha, int x, int inicio, int fim)

{

    // namedWindow("out", WINDOW_NORMAL);

    Mat canny, grelhaGray, imgBinario;

    threshold(grelha, imgBinario, 0, 255, THRESH_BINARY);

    int contaBrancos = 0;

    vector<int> posBarra;

    // percorrer toda a linha da imagem para receber os dias.

    for (int i = 50; i < imgBinario.rows; i++)

    {

        // 50 para nao passar por cima do texto no eixo dos X

        int valor = (int)imgBinario.at<uchar>(i, x);

        // cout << valor << endl;

        if (valor == 255)

            contaBrancos++;

        if (valor == 0 && contaBrancos > 0)

        {

            // encontrei a barra!

            // cout << "Encontrei a barra na posicao: " << i << endl;

            contaBrancos = 0;

            posBarra.push_back(i);
        }
    }

    vector<string> bloco;

    for (int i = 1; i < posBarra.size(); i++)

    {

        int y1 = posBarra[i - 1];

        int y2 = posBarra[i];

        if (y2 - y1 < 50)

            continue;

        ;

        Mat hora = imgBinario(Range(y1, y2), Range(inicio, fim));

        string texto = lerTexto(hora);

        bloco.push_back(texto);

        // se o bloco tiver mais de 150pixeis de altura é porque ocupa 2 blocos de horas

        // metemos outra vez para dentro do vector. Para termos 2 blocos de horas com a mesma coisa

        if (y2 - y1 > 150)

            bloco.push_back(texto);

        // imshow("hora", hora);

        // waitKey(0);
    }

    return bloco;

    // imshow("out", imgBinario);
}

int main(int argc, char **argv)

{

    Coordenadas horas = {50, 60, 180};

    Coordenadas segunda = {208, 200, 400};

    Coordenadas terca = {443, 406, 606};

    Coordenadas quarta = {678, 640, 840};

    Coordenadas quinta = {913, 875, 1075};

    Coordenadas sexta = {1148, 1110, 1310};

    namedWindow("img", WINDOW_NORMAL);

    namedWindow("grelha", WINDOW_NORMAL);

    // create json array

    json j;

    int i = 0;

    std::string path = argv[1];

    for (const auto &entry : std::filesystem::directory_iterator(path))

    {

        cout << entry.path() << endl;

        Mat img = imread(entry.path());

        Mat imgGray, grelha;

        cvtColor(img, imgGray, COLOR_BGR2GRAY);

        imshow("img", img);

        parseDaGrelha(imgGray, grelha);

        imshow("grelha", grelha);

        puts("A Calcular...");

        // //buscar as horas

        // int x = 50;

        // int inicio = 60, fim = 180;

        vector<string> horasDoDia = parseHorasOuDiaSemana(grelha, horas.x, horas.inicio, horas.fim);

        vector<string> bloco = parseHorasOuDiaSemana(grelha, segunda.x, segunda.inicio, segunda.fim);

        vector<string> tercaH = parseHorasOuDiaSemana(grelha, terca.x, terca.inicio, terca.fim);

        vector<string> quartaH = parseHorasOuDiaSemana(grelha, quarta.x, quarta.inicio, quarta.fim);

        vector<string> quintaH = parseHorasOuDiaSemana(grelha, quinta.x, quinta.inicio, quinta.fim);

        vector<string> sextaH = parseHorasOuDiaSemana(grelha, sexta.x, sexta.inicio, sexta.fim);

        imwrite("grelha-1.jpg", grelha);

        puts("\n\n");

        parseTurma(imgGray);

    }

    return 0;
}
