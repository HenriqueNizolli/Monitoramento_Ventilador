/*
 * Carrega as bibliotecas necessarias
*/
#include <SparkFun_MMA8452Q.h>   // Acelerometro
#include <Wire.h>                // Comunicação I2C
#include <UIPEthernet.h>         // Modulo Ethernet
#include <MySQL_Connection.h>    // Conecção com o banco de dados
#include <MySQL_Cursor.h>

// Variaveis ultilizadas pelo sensor optico
float rpm;

// Variaveis ultilizadas pelo acelerometro
MMA8452Q accel(0x1C);

// Variaveis ultilizadas pelo modulo ethernet
EthernetClient client;
uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

// Variaveis ultilizadas pelo conector do MySQL
IPAddress server(192,168,0,188); // ip do banco de dado
char user[] = "arduino";         // useuario do banco de dados
char password[] = "teste";       // senha do banco de dados
char INSERT_SQL[] = "INSERT INTO instrumentacao.motor_data (velocidade, eixo_x, eixo_y, eixo_z) VALUES ";  // comandos SQL que serão utilizados em todos requisições
MySQL_Connection conn((Client *)&client);

int amostra = 80;  // numero de amostra que sera coletada em 1 segundo
float tempo = (float)(1000/amostra);  // defina a cada quanto tempo sera coletado um dado
void setup()
{
    Serial.begin(115200); // inicia a comunicação serial
    Wire.begin();         // inicia a cominucação I2C
    accel.begin();        // inicia a comunicação com o acelerometro
    Ethernet.begin(mac);  // inicia o modulo ethnet e atribui o indereçõ MAC a o modulo
    conn.connect(server, 3306, user, password);  // inicia a comunicação com o banco  de dados MYSQL
}

void loop()
{
    char query[2700] = "";     // cria a query de comando SQL
    strcat(query,INSERT_SQL);  // inseri a base de do comando SQL
    for(int i = 0; i < amostra; i++)   // um for para coletar os dados
    {
        String aux = "";  // uma variavel intermediaria para facilitar a coleta dos dados
        rpm = 0;          // zera a contagem de quantas elices passaram 
        if(i == (amostra - 1))  // verifica de é a ultima coleta de dados
        {
            attachInterrupt(1,DataColect,FALLING); // ativa a interrupção no pino 
            delay(tempo); // tempo que ficara esperando para ver quantas pas passaram nesse tempo
            detachInterrupt(1);  // desativa a interrupção
            rpm = (float) (rpm / 7) * 4800; // calcula a estimativa das rotações por minuto
            aux = "(" + String(rpm,2) + "," + String((accel.getCalculatedX()), 4) + "," + String((accel.getCalculatedY()), 4) + "," + String((accel.getCalculatedZ()), 4) + ")" + ";";  // junta todos os dados na variavel intermediaria
        }
        else
        {
            attachInterrupt(1,DataColect,FALLING); // ativa a interrupção no pino 
            delay(tempo);  // tempo que ficara esperando para ver quantas pas passaram nesse tempo
            detachInterrupt(1);  // desativa a interrupção
            rpm = (float) (rpm / 7) * 4800; // calcula a estimativa das rotações por minuto
            aux = "(" + String(rpm,2) + "," + String((accel.getCalculatedX()), 3) + "," + String((accel.getCalculatedY()), 3) + "," + String((accel.getCalculatedZ()), 3) + ")" + ",";  // junta todos os dados na variavel intermediaria
        }
        strcat(query,aux.c_str()); // junta os dados coletados a query que sera enviada ao banco de dados
    }
    Serial.println(query);
    DataInsert(query);
}

void DataColect()
{
    rpm++; // acresenta um na conta de quantas elices passou na frente do sensor
}

void DataInsert(char query[])
{
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn); // aloca memoria necessaria para a criação do conector
    cur_mem->execute(query); // envia a query com os comandos SQL para o banco de dados
    delete cur_mem; // libera a memoria alocada pelo conector
}
