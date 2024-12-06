#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.0.219:1883" // Replace with your broker's address
#define CLIENTID    "MQTT_Processor"         // Client ID
#define SUB_TOPIC   "P1/MDQossay"            // Topic to subscribe to
#define QOS         1                        // Quality of Service
#define OUTPUT_FILE "/home/qmadhounrpi5/KerstExamen/data.txt"         // Output file name
#define DATE_TIME_LEN 50
#define message_LEN 300
#define OUTPUT_FORMATTED "/home/qmadhounrpi5/KerstExamen/formatted.txt"

char datum_tijd_gas[DATE_TIME_LEN], datum_stroom[DATE_TIME_LEN],tijd_stroom[DATE_TIME_LEN];
int tarief_indicator;
float actueel_stroomverbruik , actueel_spanning , totaal_dagverbruik , totaal_nachtverbruik , totaal_dagopbrengst, totaal_nachtopbrengst ,totaal_gasverbruik;
float totaal_verbruik, totaal_opbrengst, eerste_record_verbruik ,laatste_record_verbruik, eerste_record_gas , laatste_record_gas,eerste_record_opbrengst, 
laatste_record_opbrengst, verschil_verbruik, verschil_opbrengst, verschi_gas;
 int aantal_messages = 0;
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


struct meter_data
{
    char datum_stroom[DATE_TIME_LEN];
    char tijd_stroom[DATE_TIME_LEN];
    int tarief_indicator;
    float actueel_stroomverbruik;
    float actueel_spanning;
    float totaal_dagverbruik;
    float totaal_nachtverbruik;
    float totaal_dagopbrengst;
    float totaal_nachtopbrengst;
    char datum_tijd_gas[DATE_TIME_LEN];
    float totaal_gasverbruik;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------------------------------------------

// Callback for receiving messages
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *payload = (char *)message->payload;


// Print received mesaage
    printf("Received MESSAGE: <%s>\n", message);
    aantal_messages += 1;

//raw data wegschrijven naar txt file
    FILE *file = fopen(OUTPUT_FILE, "w");
    if (!file) {
        perror("Error opening file");
        return;
    }

    fprintf(file, "Statistics:\n");
    fprintf(file, "%s\n", message);
    fclose;


   

//verdelen in velden 
    int errorfields_read = sscanf(message, "%s[^-];%s[^;];%d[^;];%f[^;];%f[^;];%f[^;];%f[^;];%f[^;];%f[^;];%s[^;];%f[^\n]", 
    datum_stroom, tijd_stroom , tarief_indicator,actueel_stroomverbruik,actueel_spanning , totaal_dagverbruik,totaal_nachtverbruik, totaal_dagopbrengst, totaal_nachtopbrengst , datum_tijd_gas, totaal_gasverbruik);

 //errorhandling   
 if (errorfields_read < 9) {
        strncpy(message, "Invalid message format", message_LEN);
        return 1; 
    }

//de data verwerken 
for (int i = 0; i < aantal_messages; i++)
{ 
    totaal_verbruik = totaal_dagverbruik + totaal_nachtverbruik;
    totaal_opbrengst = totaal_dagopbrengst + totaal_nachtopbrengst;

    if (strcmp(tijd_stroom , "00:00:00") == 0)
    {
        laatste_record_verbruik = totaal_nachtverbruik[i-1];
        eerste_record_verbruik = totaal_nachtverbruik[i];

        laatste_record_opbrengst = totaal_nachtopbrengst[i-1];
        eerste_record_opbrengst = totaal_nachtopbrengst[i];

        laatste_record_gas = totaal_gasverbruik[i-1];
        eerste_record_gas = totaal_gasverbruik[i];

    }

    verschil_verbruik = laatste_record_verbruik - eerste_record_verbruik;
    verschil_opbrengst = laatste_record_opbrengst - eerste_record_opbrengst;
    verschi_gas = laatste_record_gas - eerste_record_gas;
}


//write formatted data to txt file (formatted.txt)
 FILE *file = fopen(OUTPUT_FORMATTED, "w");
    if (!file) {
        perror("Error opening file");
        return;
    }

    fprintf(file , "Startwaarden \n     DAG     Totaal verbruikt    = %f \n DAG     Totaal opbrengst    = %f \n NACHT     Totaal verbruik    = %f \n NACHT     Totaal opbrengst    = %f \n GAS     Totaal verbruik    = %f"  );

    fprintf(file , "--------------------------------------------------------------------- \n Totalen \n ------------------------------------------------------------------------- \n");
    fprintf(file , "Datum: %s \n ---------- Stroom: \n      Totaalverbruik  = %f \n        Totaalopbrengst  = %s \n Gas: \n     Totaal verbruik     = %f \n *" , datum_stroom , verschil_verbruik,verschil_opbrengst,verschi_gas);


    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}



//-----------------------------------------------------------------------------------------------------------------------------------------------------------



// Callback for connection loss
void on_connection_lost(void *context, char *cause) {
    printf("Connection lost. Cause: %s\n", cause);
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------

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
































    // Update statistics
    update_statistics(message);



// Variables to store statistics
int count = 0;                // Number of received messages
double sum = 0.0;             // Sum of all numbers
int min_value = INT_MAX;      // Minimum value
int max_value = INT_MIN;      // Maximum value




void update_statistics(int number) {
    if (number < min_value) min_value = number;
    if (number > max_value) max_value = number;
    sum += number;
    count++;
}

// Function to write statistics to a file

