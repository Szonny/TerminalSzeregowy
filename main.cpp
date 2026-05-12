#include <windows.h>
#include <iostream>
#include <sstream>
#include <winbase.h>
#include <fstream>

#define PLIK_USTAWIEN_TRANSMISJI "TransConfig.config"
#define PLIK_USTAWIEN_TIMEOUTOW "TimeOutConfig.config"

/*
DODAĆ ZAPIS USTAWIEN DO PLIKU
    //Zaawansowane funkcje - DTR,przerwania
    //EscapeCommFunction (HANDLE, DWORD); CLRDTR- usuwa DTR, SETDTR- wysyla DTR, SETRTS, CLRRTS, SETBREAK, CLRBREAK
    //FlushFileBuffers(HANDLE) - czyszczenie bufforow
    //Ping, przerwania - mechanizm Eventów i Callbacki

KONTROLA KOMUNIKACJ NA PORCIE ZNAKOWYM
1. Konfiguracja ³¹cza do komunikacji
    1.1. Wybór portu (po³¹czony ze sprawdzeniem obecnoœci portu) - OB
    1.3. Kontrola przep³ywu - OB
    - brak kontroli przep³ywu,
    - „sprzêtowa” (handshake): DTR/DSR, RTS/CTS,
    - “programowa”: XON/XOFF
    1.4. Przep³yw sterowany „rêcznie”: – mo¿liwoœæ ustawienia „na ¿yczenie” wyjœæ
    DTR lub RTS, monitoring stanu wejœæ DSR, CTS - OP.
    1.5. Wybór terminatora - OB
    - brak terminatora,
    - terminator standardowy (CR, LF, CR-LF),
    - terminator „w³asny” 1 lub 2 znakowy
2. Nadawanie - OB
3. Odbiór - OB
4. Transakcja (nadawanie i odbiór przy ustawionym ograniczeniu czasowym oczekiwania na odpowiedŸ - OP
5. PING: kontrola sprawnoœci ³¹cza wraz z pomiarem czasu „round trip delay” – OB.
6. Tryby transmisji:
        6.1. Tekstowy - OB
        Nadawanie:  Wprowadzanie znaków alfanumerycznych do bufora transmisyjnego po³¹czone z ich
                    prezentacj¹ w oknie „Nadawanie” i mo¿liwoœci¹ edycji. Po wydaniu komendy
                    „wyœlij” wys³anie bufora na ³¹cze z dopisaniem na koñcu terminatora.
        Odbiór: Prezentacja odebranych znaków alfanumerycznych w oknie „Odbiór”.
*/

char odwrocZnak(char znak)
{
    bool bity[8];

    std::cout<<'\n';
    int maska = 1;
    for(int i=0; i<8; i++)
    {
        bity[i]=znak & maska;
        maska=maska<<1;
    }
    std::cout<<'\n';

    znak=0;
    for(int i=0; i<8; i++)
        znak += (bity[i]<<(7-i));

    return znak;
}

std::string odwrocZnaki(std::string lancuch)
{
    for(int i=0; i< lancuch.length() ;i++ )
        lancuch[i]=odwrocZnak(lancuch[i]);

    return lancuch;
}

int zapiszUstawieniaWPliku(std::string plik, DCB param)
{
    std::ofstream plikWy(PLIK_USTAWIEN_TRANSMISJI);

     if(!plikWy.is_open())
        return 1;

    plikWy<<param.BaudRate<<'\n';
    plikWy<<param.ByteSize<<'\n';
    plikWy<<param.Parity<<'\n';
    plikWy<<param.StopBits<<'\n';

    plikWy.close();
    return 0;
}

int zapiszTimeoutyWPliku(std::string plik, COMMTIMEOUTS tim)
{
    std::ofstream plikWy(PLIK_USTAWIEN_TIMEOUTOW);

    if(!plikWy.is_open())
        return 1;

    plikWy<<tim.ReadIntervalTimeout<<'\n';
    plikWy<<tim.ReadIntervalTimeout<<'\n';
    plikWy<<tim.ReadTotalTimeoutConstant<<'\n';
    plikWy<<tim.ReadTotalTimeoutMultiplier<<'\n';
    plikWy<<tim.WriteTotalTimeoutConstant<<'\n';
    plikWy<<tim.WriteTotalTimeoutMultiplier<<'\n';

    plikWy.close();
    return 0;
}

int wczytajUstawienia(DCB &param, COMMTIMEOUTS &tim)
{
    std::ifstream plikU(PLIK_USTAWIEN_TRANSMISJI);
    std::ifstream plikT(PLIK_USTAWIEN_TIMEOUTOW);

    if(!plikT.is_open() || !plikU.is_open())
    {
        return 1;
    }

    plikU>>param.BaudRate;
    plikU>>param.ByteSize;
    plikU>>param.Parity;
    plikU>>param.StopBits;
    plikU.close();

    plikT>>tim.ReadIntervalTimeout;
    plikT>>tim.ReadIntervalTimeout;
    plikT>>tim.ReadTotalTimeoutConstant;
    plikT>>tim.ReadTotalTimeoutMultiplier;
    plikT>>tim.WriteTotalTimeoutConstant;
    plikT>>tim.WriteTotalTimeoutMultiplier;
    plikT.close();

    return 0;
}

void wyswietlBlad()
{
    char ostatniBlad[1024];
    std::cout<<"\n\n\n/********POWAZNY_BLAD*******\n";
    std::cout<<"Kod bledu: "<<GetLastError()<<'\n';
    FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,
            GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            ostatniBlad,1024,NULL)<<'\n';
    std::cout<<ostatniBlad<<'\n';
    std::cout<<"\\***************************\n\n\n";
}

int pobierzInt()
{
    std::string wyb;
    std::cin>>wyb;
    return atoi(wyb.c_str());
}

int pobierzIntWPrzedziale(unsigned int min,unsigned int max)
{
    std::cout<<"Podaj porzadana warosc: ";
    int liczba = pobierzInt();
    while(liczba < min || liczba > max){
        std::cout<<"\n*Podana wartosc jest niepoprawna, sproboj jeszcze raz.*\n";
        liczba = pobierzInt();
    }
    return liczba;
}

int pobierzPredkoscTransmisji(int obecna)
{
    std::cout<<"Wybierz swoja wlasna predkosc transmisji:\n \
    0- 110  \n\
    1- 300  \n\
    2- 600  \n\
    3- 1200 \n\
    4- 2400 \n\
    5- 4800 \n\
    6- 9600 \n\
    7- 14400 \n\
    8- 38400 \n\
    9- 56000 \n\
    A- 57600 \n\
    B- 115200 \n\
    C- 128000 \n\
    D- 256000 \n";
    char wyb;
    int baudrate = CBR_2400;
    std::cin>>wyb;
    switch(wyb)
    {
        case '0':
            baudrate=CBR_110;
        break;
        case '1':
            baudrate=CBR_300;
        break;
        case '2':
            baudrate=CBR_600;
        break;
        case '3':
            baudrate=CBR_1200;
        break;
        case '4':
            baudrate=CBR_2400;
        break;
        case '5':
            baudrate=CBR_4800;
        break;
        case '6':
            baudrate=CBR_9600;
        break;
        case '7':
            baudrate=CBR_14400;
        break;
        case '8':
            baudrate=CBR_38400;
        break;
        case '9':
            baudrate=CBR_56000;
        break;
        case 'a':
        case 'A':
            baudrate=CBR_57600;
        break;
        case 'b':
        case 'B':
            baudrate=CBR_115200;
        break;
        case 'c':
        case 'C':
            baudrate=CBR_128000;
        break;
        case 'd':
        case 'D':
            baudrate=CBR_256000;
        break;
        default:
            std::cout<<"Nieznana wartosc predkosci transmisji, ustawiono wartsoc domyslna\n";
    }
    return baudrate;
}

int pobierzLBitowDanych(int obecna)
{
    std::cout<<"Wybierz swoja wlasna liczbe bitow danych:\n \
    a- 5  \n\
    b- 6  \n\
    c- 7  \n\
    d- 8  \n";
    char wyb;
    int lBitow=8;
    std::cin>>wyb;
    switch(wyb)
    {
    case '5':
        case 'a':
        case 'A':
            lBitow=5;
        break;
        case '6':
        case 'b':
        case 'B':
            lBitow=6;
        break;
        case '7':
        case 'c':
        case 'C':
            lBitow=7;
        break;
        case '8':
        case 'd':
        case 'D':
            lBitow=8;
        break;
        default:
            std::cout<<"Nieznana wartosc liczby bitow\n";
    }
    return lBitow;
}

int pobierzParzystosc(int obecna)
{
    std::cout<<"Wybierz swoja wlasna kontrole bitow:\n \
    N - brak bitu parzystosci \n\
    P - bit parzystosci \n\
    O - bit nieparzystosci \n";
    char wyb;
    int bitP=NOPARITY;
    std::cin>>wyb;
    switch(wyb)
    {
        case 'n':
        case 'N':
            bitP=(int)NOPARITY;
        break;
        case 'p':
        case 'P':
            bitP=(int)EVENPARITY;
        break;
        case 'o':
        case 'O':
            bitP=(int)ODDPARITY;
        break;
        default:
            std::cout<<"Nieznany rodzaj kontroli parzystosci.\n";
    }

    return bitP;
}

int pobierzLbitowStop(int obecna)
{
    std::cout<<"Wybierz swoja wlasna dlugosc bitu stopu:\n \
    a- 1 \t czasu trwania bitu \n\
    b- 1,5  \t czsau trwania bitu \n\
    c- 2 \t czasu trwania bitu \n";
    char wyb;
    int lBitowS=ONESTOPBIT;
    std::cin>>wyb;
    switch(wyb)
    {
        case '1':
        case 'a':
        case 'A':
            lBitowS=ONESTOPBIT;
        break;
        case '5':
        case 'b':
        case 'B':
            lBitowS=ONE5STOPBITS;
        break;
        case '2':
        case 'c':
        case 'C':
            lBitowS=TWOSTOPBITS;
        break;
        default:
            std::cout<<"Nieznana wartosc liczby bitow stopu\n";
    }
    return lBitowS;
}

void wyswietlObecneUstawienia(DCB dcb)
{
        std::cout<<"Obecnie ustawiona predkosc transmisji: "<<(int)dcb.BaudRate<<" i format znaku: "<<(float)dcb.ByteSize;
        if(dcb.Parity==EVENPARITY) std::cout<<'E';
        else if(dcb.Parity==NOPARITY) std::cout<<'N';
        else if(dcb.Parity==ODDPARITY) std::cout<<'O';
        else std::cout<<'-';

        if(dcb.StopBits== ONESTOPBIT)
            std::cout<<1<<'\n';
        else if(dcb.StopBits== ONE5STOPBITS)
            std::cout<<5<<'\n';
        else    std::cout<<(int)dcb.StopBits<<'\n';
}

void wyswietlObecneTimeouty(COMMTIMEOUTS timeouts)
{
     std::cout<<"1-"<<timeouts.ReadIntervalTimeout<<"ms | 2-"<<timeouts.ReadTotalTimeoutConstant;
        std::cout<<"ms | 3-"<<timeouts.ReadTotalTimeoutMultiplier<<"ms | 4-"<<timeouts.WriteTotalTimeoutConstant;
        std::cout<<"ms | 5-"<<timeouts.WriteTotalTimeoutMultiplier<<"ms\n";
}

DCB ustawieniaTransmisji(DCB dcbParam)
{
    bool wPodmenu=true;

    std::cout<<"\n\n\n/********************************\n";
    std::cout<<"Witaj w podmenu ustawien transmisji. \n";
    while (wPodmenu)
    {
        std::cout<<"Wybierz ktory parametr transmisji chcesz ustawic.\n \
        0-Zapisz ustawienia i wroc | 1-Szybkosc Transimsji(Baudrate) | 2 - Ilosc bitow danych \n\t| 3 - Bit (nie)parzystosci | 4 - Dlugosc bitu stopu\n";
        wyswietlObecneUstawienia(dcbParam);

        char wyb;
        std::cin>>wyb;

        switch (wyb)
        {
            case '1':
                dcbParam.BaudRate=pobierzPredkoscTransmisji(dcbParam.BaudRate);
            break;
            case '2':
                dcbParam.ByteSize = pobierzLBitowDanych(dcbParam.ByteSize);
            break;
            case '3':
                dcbParam.Parity =   (WORD)pobierzParzystosc(dcbParam.Parity);

            break;
            case '4':
                dcbParam.StopBits = pobierzLbitowStop(dcbParam.StopBits);
            break;
            case '0':
            default:
                wPodmenu=false;
        }
    }

    zapiszUstawieniaWPliku(PLIK_USTAWIEN_TRANSMISJI, dcbParam);

    return dcbParam;
}

HANDLE utworzPort(int nrPortu)
{
    std::stringstream ss;
    ss<<nrPortu;
    std::string port = "\\\\.\\COM"+  ss.str();
    HANDLE hSzeregowy = CreateFile(
                                    port.c_str(),                      // Który port
                                   GENERIC_READ | GENERIC_WRITE, //Do odczytu i zapisu
                                   0, 0,
                                   OPEN_EXISTING,   //Nie twórz nowego portu
                                   FILE_ATTRIBUTE_NORMAL, //Atrybuty standardowe
                                   0);


    if(hSzeregowy == INVALID_HANDLE_VALUE)
    {
        if(GetLastError() == ERROR_FILE_NOT_FOUND)
            std::cout<<"BLAD: Nie znaleziono portu szeregowego. "<<port<<"\n";

        wyswietlBlad();
        return 0;
    }

    return hSzeregowy;
}


void wyswietlDostepnePorty()
{
    std::cout<<"Dostepne Porty:\n";
    for(int i=0 ; i<=255; i++)
    {
        std::stringstream ss;
        ss<<i;
        std::string port = "\\\\.\\COM"+  ss.str();
        HANDLE hSzeregowy = CreateFile(
                                    port.c_str(),                      // Który port
                                   GENERIC_READ | GENERIC_WRITE, //Do odczytu i zapisu
                                   0, 0,
                                   OPEN_EXISTING,   //Nie twórz nowego portu
                                   FILE_ATTRIBUTE_NORMAL, //Atrybuty standardowe
                                   0);
        if(hSzeregowy != INVALID_HANDLE_VALUE)
        {
            std::cout<<port<<'\n';
            CloseHandle(hSzeregowy);
        }
    }
}

int ustawNrPortu(int nrPortu)
{
    wyswietlDostepnePorty();

    std::cout<<"Obecny numer portu to: "<<nrPortu<<"\nPodaj nowy numer portu (SAM NUMER np. 10): ";

    nrPortu = pobierzInt();
    HANDLE test = utworzPort(nrPortu);
    if(test!=0)
        CloseHandle(test);
    return nrPortu;
}

int wysylanieWiadomosci(int nrPortu, DCB parametry ,COMMTIMEOUTS timeouts)
{
    HANDLE hSzeregowy = utworzPort(nrPortu);
    if(!hSzeregowy)
        return 1;

    if(!SetCommState(hSzeregowy, &parametry))
    {
        std::cout<<"BLAD: Nie udalo sie ustawic parametrow poru szeregowego.\n";
        wyswietlBlad();
        return 1;
    }

    if(!SetCommTimeouts(hSzeregowy, &timeouts))
    {
        std::cout<<" \nBLAD: Nie udalo sie ustawic domyslnych limitow czasowych\n";
        wyswietlBlad();
        return 1;
    }

    DWORD odczytane=0;

    std::cout<<"Twoja wiadomosc (maks 100 znkow): ";
    std::stringstream wiadomosc;
    std::string buff;
    std::cin>>buff;
    wiadomosc<<buff;
    std::getline(std::cin,buff);
    wiadomosc<<buff;

    //Czy cstr ma ta sama dlugosc co str.length? -> znak /0 uwzgledniony?
    //if(!WriteFile(hSzeregowy,(odwrocZnaki(wiadomosc.str())).c_str(), wiadomosc.str().length(), &odczytane, NULL))     //WriteFile robi zapis
    if(!WriteFile(hSzeregowy,(wiadomosc.str()).c_str(), wiadomosc.str().length(), &odczytane, NULL))     //WriteFile robi zapis
    {
        std::cout<<"BLAD: Nie udalo sie wyslac wiadomosci\n";
        wyswietlBlad();
        return 1;
    }
    CloseHandle(hSzeregowy);

    return 0;
}

int nasluchujWiadomosci(int nrPortu, DCB parametry ,COMMTIMEOUTS timeouts)
{
    HANDLE hSzeregowy = utworzPort(nrPortu);
    if(!hSzeregowy)
        return 1;

    char buff[1000];
    DWORD odczytane=0;

    if(!SetCommState(hSzeregowy, &parametry))
    {
        std::cout<<"BLAD: Nie udalo sie ustawic parametrow poru szeregowego.\n";
        wyswietlBlad();
        return 1;
    }

    if(!SetCommTimeouts(hSzeregowy, &timeouts))
    {
        std::cout<<" \nBLAD: Nie udalo sie ustawic domyslnych limitow czasowych\n";
        wyswietlBlad();
        return 1;
    }

    if(!ReadFile(hSzeregowy,buff,1000, &odczytane, NULL))     //WriteFile robi zapis
    {
        std::cout<<"BLAD: Nie udalo sie odczytac wiadomosci\n";
        wyswietlBlad();
        return 1;
    }
    if(odczytane <1)
        std::cout<<"Przekroczono czas polaczenia\n";
    else
    {
        std::cout<<"Odczytano wiadomosc: \n";
        std::string odczyt(buff);
        //printf(odwrocZnaki(odczyt).c_str());
        printf(odczyt.c_str());
        std::cout<<"\n";
    }

    CloseHandle(hSzeregowy);
    return 0;
}

COMMTIMEOUTS ustawieniaCzasowOczekiwania(COMMTIMEOUTS timeouts)
{
    bool wPodmenu=true;

    std::cout<<"\n\n\n/********************************\n";
    std::cout<<"Witaj w podmenu ustawien maksymalnych czasow oczekiwania podczas transmisji. \n";
    while (wPodmenu)
    {
        std::cout<<"Wybierz ktory parametr transmisji chcesz ustawic.\n"
        <<"0-Zapisz ustawienia i wroc | 1-Czas miedzy odczytanymi znakami |"
        <<"2 - Czas oczekiania na odczyt przed porzuceniem transimsji |"
        <<"3 - Czas oczekiwania na kazdy odczytany bajt | 4 - Czas oczekiwania na operacja zapisu"
        <<"5 - Czas oczekiwania na kazdy zapisywany bajt??\n";
        std::cout<<"Obecnie ustawione: ";
        wyswietlObecneTimeouty(timeouts);
        char wyb;
        std::cin>>wyb;

        switch (wyb)
        {
            case '1':
                timeouts.ReadIntervalTimeout    =       pobierzIntWPrzedziale(0, MAXDWORD);
            break;
            case '2':
                timeouts.ReadTotalTimeoutConstant =     pobierzIntWPrzedziale(0, MAXDWORD);
            break;
            case '3':
                timeouts.ReadTotalTimeoutMultiplier =   pobierzIntWPrzedziale(0, MAXDWORD);
            break;
            case '4':
                timeouts.WriteTotalTimeoutConstant =    pobierzIntWPrzedziale(0, MAXDWORD);
            break;
            case '5':
                timeouts.WriteTotalTimeoutMultiplier =  pobierzIntWPrzedziale(0, MAXDWORD);
            break;
            case '0':
            default:
                wPodmenu=false;
        }
    }

    zapiszTimeoutyWPliku(PLIK_USTAWIEN_TIMEOUTOW,timeouts);

    return timeouts;
}

void wysylanieCiagleWiadomosci(int nrPortu, DCB dcbParamSzereg, COMMTIMEOUTS timeouts)
{
    bool wysylamy = true;
    while(wysylamy)
    {
        if(wysylanieWiadomosci(nrPortu, dcbParamSzereg, timeouts))
            break;
        std::cout<<"Wysyłamy dalej? (Y/N): ";
        char wyb;
        std::cin>>wyb;
        if (wyb == 'N' || wyb == 'n' || wyb == '0')
            wysylamy=false;
    }
}

void nasluchujCiagleWiadomosci(int nrPortu, DCB dcbParamSzereg, COMMTIMEOUTS timeouts)
{
    bool wysylamy = true;
    while(wysylamy)
    {
        if(nasluchujWiadomosci(nrPortu, dcbParamSzereg, timeouts))
            break;
        std::cout<<"Nasluchujemy dalej? (Y/N): ";
        char wyb;
        std::cin>>wyb;
        if (wyb == 'N' || wyb == 'n' || wyb == '0')
            wysylamy=false;
    }
}

int main( int argc, char** argv)
{
    DCB dcbParamSzereg = {0};
    COMMTIMEOUTS timeouts = {0};
    int nrPortu=1;

    if( wczytajUstawienia(dcbParamSzereg, timeouts))
    {
        std::cout<<"\nNie znaleziono poprzednich ustawien!\n";

        dcbParamSzereg.BaudRate=CBR_9600;
        dcbParamSzereg.ByteSize=8;
        dcbParamSzereg.Parity=NOPARITY;
        dcbParamSzereg.StopBits=ONESTOPBIT;
        timeouts.ReadIntervalTimeout=1; //maks czas miedzy znakami.     milisekundy
        timeouts.ReadTotalTimeoutConstant=1; //jak dlugo czekac przed porzuceniem
        timeouts.ReadTotalTimeoutMultiplier=1; //Czas oczekiwania na kazdy bajt operacji
        timeouts.WriteTotalTimeoutConstant=1;  //dla zapisu, ale to samo
        timeouts.WriteTotalTimeoutMultiplier=1;
    }

    std::cout<<"Witaj w epickim terminalu\n";

    bool maDzialac=true;
    while(maDzialac)
    {
        std::cout<<"\n\n\n/********************************\n";
        std::cout<<"Obecny numer portu: COM"<<nrPortu<<'\n';
        wyswietlObecneUstawienia(dcbParamSzereg);
        std::cout<<"Obecne timeouty: "; wyswietlObecneTimeouty(timeouts);

        char wyb;
        std::cout<<"\nWybierz co chcesz zrobic: \n \t 0-wyjsc z programu 1-Ustawic port 2-Ustawienia transmisji \n \
        3-Ustawienia czasow oczekiwania 4-wyslac wiadomosc 5-odebrac wiadomosc\n"
        <<"6-Wysylac wiadomosci 7-Odbierac wiadomosci\n";
        std::cout<<"\\********************************\n";
        std::cout<<"Wybor>";
        std::cin>>wyb;

        if(wyb=='0')
            maDzialac=false;
        if(wyb=='1')
            nrPortu = ustawNrPortu(nrPortu);
        if(wyb=='2')
            dcbParamSzereg=ustawieniaTransmisji(dcbParamSzereg);
        if(wyb=='3')
            timeouts=ustawieniaCzasowOczekiwania(timeouts);
        if(wyb=='4')
            wysylanieWiadomosci(nrPortu,dcbParamSzereg, timeouts);
        if(wyb=='5')
            nasluchujWiadomosci(nrPortu,dcbParamSzereg, timeouts);
        if(wyb=='6')
            wysylanieCiagleWiadomosci(nrPortu,dcbParamSzereg, timeouts);
        if(wyb=='7')
            nasluchujCiagleWiadomosci(nrPortu,dcbParamSzereg, timeouts);
    }
    return 0;

}
