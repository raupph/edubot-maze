#include "EdubotLib.hpp"
#include <iostream>

// Constantes do robô
const double FORWARD_SPEED = 0.4;
const double CRITICAL_FRONT_DISTANCE = 0.15;
const double WALL_LOST_DISTANCE = 1.0;
const double EXIT_DISTANCE = 1.5; // Distância para considerar que o robô saiu do labirinto
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

        if (currentState == MOVING_STRAIGHT &&
            edubot.getSonar(3) > EXIT_DISTANCE &&  // Frente livre
            edubot.getSonar(6) > EXIT_DISTANCE &&  // Direita livre
            edubot.getSonar(0) > EXIT_DISTANCE) {  // Esquerda livre 
            
            std::cout << "\n*****************************************" << std::endl;
            std::cout << "SAIDA DO LABIRINTO DETECTADA!" << std::endl;
            std::cout << "Parando o robo em 2 segundos..." << std::endl;
            std::cout << "*****************************************\n" << std::endl;
            
            edubot.stop();
            edubot.sleepMilliseconds(2000); // Aguarda os 2 segundos solicitados
            break; // Sai do loop while para finalizar o programa
        }
        // --- FIM DA NOVA CONDIÇÃO ---

        switch (currentState) {
            case MOVING_STRAIGHT: {
                std::cout << "[Estado: LINHA RETA]" << std::endl;
                if (edubot.getSonar(3) < CRITICAL_FRONT_DISTANCE) {
                    std::cout << "Obstaculo encontrado! Iniciando seguimento de parede (Pledge)..." << std::endl;
                    edubot.stop();
                    pledgeCounter = -90.0;
                    edubot.rotate(-90.0);
                    edubot.sleepMilliseconds(1000);
                    currentState = WALL_FOLLOWING;
                } else {
                    edubot.move(FORWARD_SPEED);
                }
                break;
            }

            case WALL_FOLLOWING: {
                if (pledgeCounter == 0.0 && edubot.getSonar(3) > WALL_LOST_DISTANCE) {
                    std::cout << "Contador Pledge zerado e caminho livre! Retomando linha reta." << std::endl;
                    edubot.stop();
                    currentState = MOVING_STRAIGHT;
                    break;
                }
                
                double front_sonar = edubot.getSonar(3);
                double right_sonar = edubot.getSonar(6);

                std::cout << "[Estado: SEGUINDO PAREDE] Pledge: " << pledgeCounter << ", Frontal: " << front_sonar << "m, Direito: " << right_sonar << "m" << std::endl;

                if (right_sonar > WALL_LOST_DISTANCE) {
                    std::cout << "Quina externa detectada. Fazendo curva aberta..." << std::endl;
                    edubot.sleepMilliseconds(600);
                    edubot.stop();
                    edubot.rotate(90.0);
                    pledgeCounter += 90.0;
                    edubot.sleepMilliseconds(1000);
                    edubot.move(FORWARD_SPEED);
                    edubot.sleepMilliseconds(750);
                }
                else if (front_sonar < CRITICAL_FRONT_DISTANCE) {
                    std::cout << "Quina interna detectada. Virando a esquerda..." << std::endl;
                    edubot.stop();
                    edubot.rotate(-90.0);
                    pledgeCounter -= 90.0;
                    edubot.sleepMilliseconds(1000);
                }
                else {
                    edubot.move(FORWARD_SPEED);
                }
                break;
            }
        }

        edubot.sleepMilliseconds(LOOP_DELAY_MS);
    }

    std::cout << "Fim da missao." << std::endl;
    edubot.disconnect();

    return 0;
}