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
#include "lib/display_init.h"  // inicializa SSD1306

// Configurações Wi-Fi
#define WIFI_SSID     "RONALDO"
#define WIFI_PASSWORD "36394578"

// Configurações da Matriz
#define BRILHO_PADRAO 30
#define PINO_MATRIZ   7

// Estados dos quadrantes
static bool q1_on = false, q2_on = false, q3_on = false, q4_on = false;

// Matriz base: cruz vermelha de brilho baixo
static int base_mat[5][5][3] = {
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,30,30},{0,0,0},{0,0,0}},
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,30,30},{0,0,0},{0,0,0}},
    {{BRILHO_PADRAO,30,30},{BRILHO_PADRAO,30,30},{BRILHO_PADRAO,30,30},{BRILHO_PADRAO,30,30},{BRILHO_PADRAO,30,30}},
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,30,30},{0,0,0},{0,0,0}},
    {{0,0,0},{0,0,0},{BRILHO_PADRAO,30,30},{0,0,0},{0,0,0}}
};

// Protótipos
void update_matrix(void);
void update_display(void);
void user_request(char *request);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

// Atualiza o display OLED com potência total
void update_display() {
    int potencia = (q1_on + q2_on + q3_on + q4_on) * 10;
    char buf[32];
    ssd1306_fill(&ssd, false);
    snprintf(buf, sizeof(buf), "Potencia: %d W", potencia);
    ssd1306_draw_string(&ssd, buf, 0, 0);
    ssd1306_send_data(&ssd);
}

// Atualiza a matriz de LEDs e chama o display
void update_matrix() {
    int mat[5][5][3];
    memcpy(mat, base_mat, sizeof(mat));

    if (q1_on) for (int y=0;y<2;y++) for (int x=0;x<2;x++) mat[y][x][0]=mat[y][x][1]=BRILHO_PADRAO;
    if (q2_on) for (int y=0;y<2;y++) for (int x=3;x<5;x++) mat[y][x][0]=mat[y][x][1]=BRILHO_PADRAO;
    if (q3_on) for (int y=3;y<5;y++) for (int x=0;x<2;x++) mat[y][x][0]=mat[y][x][1]=BRILHO_PADRAO;
    if (q4_on) for (int y=3;y<5;y++) for (int x=3;x<5;x++) mat[y][x][0]=mat[y][x][1]=BRILHO_PADRAO;

    desenhaMatriz(mat);
    set_brilho(BRILHO_PADRAO);
    bf();
    update_display();
}

// Trata requisição e alterna quadrantes
void user_request(char *request) {
    if (strstr(request, "GET /quadrante1")) { q1_on = !q1_on; update_matrix(); }
    else if (strstr(request, "GET /quadrante2")) { q2_on = !q2_on; update_matrix(); }
    else if (strstr(request, "GET /quadrante3")) { q3_on = !q3_on; update_matrix(); }
    else if (strstr(request, "GET /quadrante4")) { q4_on = !q4_on; update_matrix(); }
}

int main() {
    stdio_init_all();
    controle(PINO_MATRIZ);
    display();       // inicializa OLED
    update_matrix(); // pinta estado inicial

    if (cyw43_arch_init()) return -1;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID,WIFI_PASSWORD,CYW43_AUTH_WPA2_AES_PSK,20000)) return -1;
    
    printf("Conectado ao Wi-Fi\n");
    if (netif_default) printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));

    struct tcp_pcb *server = tcp_new();
    if (!server || tcp_bind(server, IP_ADDR_ANY, 80)!=ERR_OK) return -1;
    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    while (true) {
        cyw43_arch_poll();
        sleep_ms(100);
    }
    cyw43_arch_deinit();
    return 0;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    if (p->len>0 && strncmp(p->payload,"GET",3)==0) {
        char *request = malloc(p->len+1);
        memcpy(request,p->payload,p->len);
        request[p->len]='\0';
        user_request(request);

        int potencia = (q1_on + q2_on + q3_on + q4_on) * 10;

        char html[2048];
        snprintf(html,sizeof(html),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
            "<title>Light Control</title>"
            "<style>body{font-family:Arial;text-align:center;}button{padding:15px;margin:10px;}</style>"
            "</head><body>"
            "<h1>Controle de Iluminação</h1>"
            "<form action=\"/quadrante1\"><button>Toggle Quarto 1</button></form>"
            "<form action=\"/quadrante2\"><button>Toggle Quarto 2</button></form>"
            "<form action=\"/quadrante3\"><button>Toggle Quarto 3</button></form>"
            "<form action=\"/quadrante4\"><button>Toggle Quarto 4</button></form>"
            "<h2>Potência total: %d W</h2>"
            "</body></html>", potencia);

        tcp_write(tpcb,html,strlen(html),TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        free(request);
    }
    pbuf_free(p);
    return ERR_OK;
}
