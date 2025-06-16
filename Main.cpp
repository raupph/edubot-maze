#include "EdubotLib.hpp"
#include <iostream>

// Constantes do robô
const double FORWARD_SPEED = 0.4;
const double CRITICAL_FRONT_DISTANCE = 0.15;
const double WALL_LOST_DISTANCE = 1.0; // Distância para detectar uma curva à direita (quina externa)
const int LOOP_DELAY_MS = 50;

// Estados para o Algoritmo de Pledge
enum RobotState { MOVING_STRAIGHT, WALL_FOLLOWING };

int main() {
    EdubotLib edubot;

    if (!edubot.connect()) {
        std::cout << "Nao foi possivel conectar ao robo." << std::endl;
        return -1;
    }

    std::cout << "Conectado ao robo! Iniciando o Algoritmo de Pledge..." << std::endl;
    edubot.sleepMilliseconds(2000);

    RobotState currentState = MOVING_STRAIGHT;
    double pledgeCounter = 0.0; // Contador de giros em graus

    while (edubot.isConnected()) {
        switch (currentState) {
            case MOVING_STRAIGHT: {
                std::cout << "[Estado: LINHA RETA]" << std::endl;
                // 1. Verifica se há um obstáculo à frente
                if (edubot.getSonar(3) < CRITICAL_FRONT_DISTANCE) {
                    std::cout << "Obstaculo encontrado! Iniciando seguimento de parede (Pledge)..." << std::endl;
                    edubot.stop();
                    // Zera o contador e vira à esquerda para colocar a "mão na parede" (direita)
                    pledgeCounter = -90.0;
                    edubot.rotate(-90.0);
                    edubot.sleepMilliseconds(1000);
                    currentState = WALL_FOLLOWING;
                } else {
                    // 2. Se não há obstáculo, continua em frente
                    edubot.move(FORWARD_SPEED);
                }
                break;
            }

            case WALL_FOLLOWING: {
                // Condição de saída do Pledge: contador zerado E caminho livre
                if (pledgeCounter == 0.0 && edubot.getSonar(3) > WALL_LOST_DISTANCE) {
                    std::cout << "Contador Pledge zerado e caminho livre! Retomando linha reta." << std::endl;
                    edubot.stop();
                    currentState = MOVING_STRAIGHT;
                    break;
                }
                
                double front_sonar = edubot.getSonar(3);
                double right_sonar = edubot.getSonar(6);

                std::cout << "[Estado: SEGUINDO PAREDE] Pledge: " << pledgeCounter << ", Frontal: " << front_sonar << "m, Direito: " << right_sonar << "m" << std::endl;

                // Lógica de seguir parede (igual à anterior, mas agora atualiza o contador)

                // A. Curva para a direita (quina externa)
                if (right_sonar > WALL_LOST_DISTANCE) {
                    std::cout << "Quina externa detectada. Fazendo curva aberta..." << std::endl;
                    // Sua manobra para passar a quina
                    edubot.sleepMilliseconds(600);
                    edubot.stop();
                    edubot.rotate(90.0);
                    pledgeCounter += 90.0; // ATUALIZA O CONTADOR
                    edubot.sleepMilliseconds(1000);
                    edubot.move(FORWARD_SPEED);
                    edubot.sleepMilliseconds(750);
                }
                // B. Curva para a esquerda (quina interna)
                else if (front_sonar < CRITICAL_FRONT_DISTANCE) {
                    std::cout << "Quina interna detectada. Virando a esquerda..." << std::endl;
                    edubot.stop();
                    edubot.rotate(-90.0);
                    pledgeCounter -= 90.0; // ATUALIZA O CONTADOR
                    edubot.sleepMilliseconds(1000);
                }
                // C. Segue reto ao longo da parede
                else {
                    edubot.move(FORWARD_SPEED);
                }
                break;
            }
        }

        edubot.sleepMilliseconds(LOOP_DELAY_MS);
    }

    std::cout << "Desconectado. Fim do programa." << std::endl;
    edubot.disconnect();

    return 0;
}