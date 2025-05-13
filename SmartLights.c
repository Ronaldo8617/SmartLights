#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lib/matrixws.h"

// Configurações Wi-Fi
#define WIFI_SSID       "XXXX"       // Nome da rede Wi-Fi
#define WIFI_PASSWORD   "XXXX"      // Senha da rede Wi-Fi

// Configurações da Matriz
#define BRILHO_PADRAO   30              // Brilho padrão dos LEDs (0-255)
#define PINO_MATRIZ     7               // Pino GPIO conectado à matriz de LEDs

// Estados dos quadrantes
static bool q1_on = false, q2_on = false, q3_on = false, q4_on = false;  // Controlam o estado de cada quadrante

// Matriz base: cruz vermelha de brilho baixo
static int base_mat[5][5][3] = {        // Matriz 5x5 com valores RGB para cada LED
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,0,0},{0,0,0},{0,0,0}},
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,0,0},{0,0,0},{0,0,0}},
    {{BRILHO_PADRAO,0,0},{BRILHO_PADRAO,0,0},{BRILHO_PADRAO,0,0},{BRILHO_PADRAO,0,0},{BRILHO_PADRAO,0,0}},
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,0,0},{0,0,0},{0,0,0}},
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,0,0},{0,0,0},{0,0,0}}
};

// Protótipos de funções
void update_matrix(void);  // Atualiza o estado da matriz de LEDs
void user_request(char *request);  // Processa requisições do usuário
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);  // Manipula novas conexões TCP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);  // Processa dados recebidos

// Atualiza a matriz física de LEDs
void update_matrix() {
    int mat[5][5][3];
    memcpy(mat, base_mat, sizeof(mat));  // Copia a matriz base

    // Ativa quadrantes em amarelo (vermelho + verde)
    if (q1_on) for (int y=0; y<2; y++) for (int x=0; x<2; x++) { mat[y][x][0]=BRILHO_PADRAO; mat[y][x][1]=BRILHO_PADRAO; }
    if (q2_on) for (int y=0; y<2; y++) for (int x=3; x<5; x++){ mat[y][x][0]=BRILHO_PADRAO; mat[y][x][1]=BRILHO_PADRAO; }
    if (q3_on) for (int y=3; y<5; y++) for (int x=0; x<2; x++) { mat[y][x][0]=BRILHO_PADRAO; mat[y][x][1]=BRILHO_PADRAO; }
    if (q4_on) for (int y=3; y<5; y++) for (int x=3; x<5; x++) { mat[y][x][0]=BRILHO_PADRAO; mat[y][x][1]=BRILHO_PADRAO; }

    desenhaMatriz(mat);  // Desenha a matriz na memória
    set_brilho(BRILHO_PADRAO);  // Aplica o brilho configurado
    bf();  // Atualiza a matriz física (buffer flip)
}

// Processa requisições HTTP recebidas
void user_request(char *request) {
    // Alterna estados dos quadrantes conforme a requisição
    if (strstr(request, "GET /quadrante1") != NULL) { q1_on = !q1_on; update_matrix(); }
    else if (strstr(request, "GET /quadrante2") != NULL) { q2_on = !q2_on; update_matrix(); }
    else if (strstr(request, "GET /quadrante3") != NULL) { q3_on = !q3_on; update_matrix(); }
    else if (strstr(request, "GET /quadrante4") != NULL) { q4_on = !q4_on; update_matrix(); }
}

// Função principal
int main() {
    stdio_init_all();  // Inicializa SDK do Raspberry Pi Pico
    controle(PINO_MATRIZ);  // Configura o pino da matriz de LEDs
    update_matrix();  // Atualiza estado inicial da matriz

    // Inicialização do Wi-Fi
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar Wi-Fi\n");
        return -1;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);  // Desliga LED interno
    cyw43_arch_enable_sta_mode();  // Modo estação Wi-Fi

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return -1;
    }
    printf("Conectado ao Wi-Fi\n");
    if (netif_default) printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));

    // Configuração do servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server) return -1;
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) return -1;  // Porta 80 para HTTP
    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");

    // Loop principal de polling
    while (true) {
        cyw43_arch_poll();  // Mantém a conexão Wi-Fi ativa
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}

// Função de callback para novas conexões
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);  // Configura callback para recebimento de dados
    return ERR_OK;
}

// Função de processamento de dados recebidos
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Verifica se é uma requisição GET
    if (p->len > 0 && strncmp(p->payload, "GET", 3) == 0) {
        char *request = malloc(p->len + 1);
        memcpy(request, p->payload, p->len);
        request[p->len] = '\0';
        printf("Request: %s\n", request);

        user_request(request);  // Processa a requisição do usuário

        // Gera HTML de resposta
        char html[2048];
        snprintf(html, sizeof(html),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
            "<title>Light Control</title>"
            "<style>body{font-family:Arial;text-align:center;}button{padding:15px;margin:10px;}</style>"
            "</head><body>"
            "<h1>Controle de Iluminação </h1>"
            "<form action=\"/quadrante1\"><button>Toggle Quarto 1</button></form>"
            "<form action=\"/quadrante2\"><button>Toggle Quarto 2</button></form>"
            "<form action=\"/quadrante3\"><button>Toggle Quarto 3</button></form>"
            "<form action=\"/quadrante4\"><button>Toggle Quarto 4</button></form>"
            "</body></html>"
        );

        tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);  // Envia resposta HTTP
        tcp_output(tpcb);
        free(request);
        pbuf_free(p);
        return ERR_OK;
    }
}