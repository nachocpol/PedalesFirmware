#include "HTTPHelper.h"
#include "Config.h"

#include "esp_http_client.h"
#include "esp_log.h"

#include "string.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

void PostInfluxDBMessage(const char* data)
{
    ESP_LOGI(k_LogTag, "Sending: %s", data);

    char url[100] = {};
    sprintf(
        url, 
        "http://%s:8086/api/v2/write?org=%s&bucket=%s&precision=ns",
        k_RaspberryIP, k_InfluxOrg, k_InfluxBucket
    );

    esp_http_client_config_t  config = {
        .url = url
    };

    /*
    curl --request POST \
    "http://localhost:8086/api/v2/write?org=YOUR_ORG&bucket=YOUR_BUCKET&precision=ns" \
        --header "Authorization: Token YOUR_API_TOKEN" \
        --header "Content-Type: text/plain; charset=utf-8" \
        --header "Accept: application/json" \
        --data-binary '
            airSensors,sensor_id=TLM0201 temperature=73.97038159354763,humidity=35.23103248356096,co=0.48445310567793615 1630424257000000000
            airSensors,sensor_id=TLM0202 temperature=75.30007505999716,humidity=35.651929918691714,co=0.5141876544505826 1630424257000000000
        '
    */
   char tokenStr[180] = {};
   sprintf(tokenStr, "Token %s", k_InfluxToken);

    esp_http_client_handle_t client = esp_http_client_init(&config);
    {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_url(client, url);
        esp_http_client_set_header(client, "Authorization", tokenStr);
        esp_http_client_set_header(client, "Content-Type", "text/plain; charset=utf-8");
        esp_http_client_set_header(client, "Accept", "tapplication/json");
        esp_err_t result = esp_http_client_open(client, strlen(data));
        if(result != ESP_OK)
        {
            ESP_LOGI(k_LogTag, "Failed to open http client");
        }
        else
        {
            // ESP_LOGI(k_LogTag, "Http client opened!");
            int wlen = esp_http_client_write(client, data, strlen(data));
            if(wlen < 0)
            {
                ESP_LOGI(k_LogTag, "Failed to write data to http client");
            }
            int contentLength = esp_http_client_fetch_headers(client);
            if(contentLength < 0)
            {
                ESP_LOGI(k_LogTag, "Failed to fetch headers");
            }
            else
            {
                char reponseData[300];
                int dataRead = esp_http_client_read_response(client, reponseData, 300);
                if(dataRead >= 0)
                {
                    int response = esp_http_client_get_status_code(client);
                    if(response >= 200 && response < 400)
                    {
                        // Points written successfully
                    }
                    else
                    {
                        ESP_LOGI(
                            k_LogTag, "HTTP POST Status = %d, content_length = %"PRIu64,
                            esp_http_client_get_status_code(client),
                            esp_http_client_get_content_length(client)
                        );
                        ESP_LOGI(k_LogTag, "%s", reponseData);
                    }
                }
                else
                {
                    ESP_LOGI(k_LogTag, "Failed to read response");
                }
            }
        }
    }
    esp_http_client_cleanup(client);
}