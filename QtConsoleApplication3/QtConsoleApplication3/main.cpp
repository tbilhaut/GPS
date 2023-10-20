#include <QCoreApplication>
#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <fstream>
#include <iomanip>

struct GPSData {
	double latitude;
	double longitude;
	std::string time;
};

bool ParseGPGGA(const std::string &sentence, GPSData &data) {
	if (sentence.find("$GPGGA") == std::string::npos) {
		return false;
	}

	std::istringstream stream(sentence);
	std::string token;
	std::vector<std::string> tokens;

	while (std::getline(stream, token, ',')) {
		tokens.push_back(token);
	}

	if (tokens.size() < 15) { // Ajustez ce nombre en fonction du nombre de champs dans votre trame NMEA
		return false;
	}

	std::string time = tokens[1];
	double latitude = std::stod(tokens[2]);
	double longitude = std::stod(tokens[4]);

	data.latitude = floor(latitude / 100) + fmod(latitude, 100) / 60;
	data.longitude = floor(longitude / 100) + fmod(longitude, 100) / 60;
	data.time = time;

	return true;
}


int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);

	QSerialPort serial;
	serial.setPortName("COM9"); // Définir le port série approprié
	serial.setBaudRate(QSerialPort::Baud9600); // Définir le baud rate

	if (!serial.open(QIODevice::ReadOnly)) {
		std::cout << "Erreur lors de l'ouverture du port série." << std::endl;
		return 1;
	}

	int numberOfReadings = 0; // Définir le nombre de lectures maximum
	const int maxReadings = 10; // Modifier ce nombre selon vos besoins

	while (numberOfReadings < maxReadings) { // Utiliser cette condition pour sortir de la boucle après un certain nombre de lectures
		if (serial.waitForReadyRead(-1)) {
			QByteArray requestData = serial.readAll();
			std::string arduinoData = requestData.toStdString();

			GPSData gpsData;
			if (ParseGPGGA(arduinoData, gpsData)) {
				std::cout << "Time: " << gpsData.time << std::endl;
				std::cout << "Latitude: " << gpsData.latitude << " degrees" << std::endl;
				std::cout << "Longitude: " << gpsData.longitude << " degrees" << std::endl;

				// Écrire les données dans le fichier de sortie
				std::ofstream outputFile;
				outputFile.open("output.txt", std::ios_base::app); // Ouvrir le fichier en mode append
				outputFile << "Time: " << gpsData.time << std::endl;
				outputFile << "Latitude: " << gpsData.latitude << " degrees" << std::endl;
				outputFile << "Longitude: " << gpsData.longitude << " degrees" << std::endl;
				outputFile.close(); // Fermer le fichier de sortie

				numberOfReadings++; // Incrémenter le nombre de lectures
			}
			else {
				std::cout << "Invalid NMEA sentence or unsupported type." << std::endl;
			}
		}
	}

	return a.exec();
}
