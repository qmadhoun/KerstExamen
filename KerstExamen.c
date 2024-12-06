#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://172.20.10.4:1883" // Replace with your broker's address
#define CLIENTID    "MQTT_Processor"         // Client ID
#define SUB_TOPIC   "FD/ERROR_IN"            // Topic to subscribe to
#define QOS         1                        // Quality of Service
#define OUTPUT_FILE "statistics.txt"         // Output file name

// Variables to store statistics
int count = 0;                // Number of received messages
double sum = 0.0;             // Sum of all numbers
int min_value = INT_MAX;      // Minimum value
int max_value = INT_MIN;      // Maximum value

// Function to update statistics
void update_statistics(int number) {
    if (number < min_value) min_value = number;
    if (number > max_value) max_value = number;
    sum += number;
    count++;
}

// Function to write statistics to a file
void write_statistics_to_file() {
    FILE *file = fopen(OUTPUT_FILE, "w");
    if (!file) {
        perror("Error opening file");
        return;
    }

    double average = (count > 0) ? sum / count : 0.0;
    fprintf(file, "Statistics:\n");
    fprintf(file, "Min: %d\n", min_value);
    fprintf(file, "Max: %d\n", max_value);
    fprintf(file, "Average: %.2f\n", average);
    fprintf(file, "Count: %d\n", count);
    fclose(file);

    printf("Statistics written to %s\n", OUTPUT_FILE);
}

// Callback for receiving messages
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *payload = (char *)message->payload;
    int number = atoi(payload); // Convert payload to integer

    // Print received number
    printf("Received number: %d\n", number);

    // Update statistics
    update_statistics(number);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Callback for connection loss
void on_connection_lost(void *context, char *cause) {
    printf("Connection lost. Cause: %s\n", cause);
}

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Create MQTT client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Set up connection options
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Set callbacks
    MQTTClient_setCallbacks(client, NULL, on_connection_lost, on_message, NULL);

    // Connect to broker
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return EXIT_FAILURE;
    }

    // Subscribe to topic
    printf("Subscribed to topic: %s\n", SUB_TOPIC);
    MQTTClient_subscribe(client, SUB_TOPIC, QOS);

    // Wait for messages
    printf("Waiting for messages...\n");
    while (1) {
        // Sleep to reduce CPU usage (optional)
        usleep(100000);

        // Every 10 messages or so, write statistics to the file
        if (count > 0 && count % 10 == 0) {
            write_statistics_to_file();
        }
    }

    // Clean up (not reached in this example)
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return EXIT_SUCCESS;
}
