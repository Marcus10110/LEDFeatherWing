#include "web_server.h"
#include <esp_http_server.h>
#include <Arduino.h>
#include "generated.h"
#include "settings.h"
#include "utilities.h"
#include "sync.h"
namespace WebServer
{
    const char* BroadcastHeaderName = "x-broadcast";
    namespace
    {
        httpd_handle_t ServerHandle = nullptr;
        std::function<void()> BroadcastSettingsCallback;
        std::function<void()> QueueDiscoveryCallback;

        esp_err_t HandleGetHomepage( httpd_req_t* req )
        {
            Serial.println( "get index" );
            httpd_resp_send( req, Content::index, HTTPD_RESP_USE_STRLEN );
            return ESP_OK;
        }

        esp_err_t HandleGetSettings( httpd_req_t* req )
        {
            Serial.println( "get settings" );
            /* Send a simple response */
            String settings_string;
            Settings::SerializeSettings( Settings::Settings, settings_string, []( JsonDocument* doc ) {
                // add neightbor names!
                int neighbor_count = 0;
                auto neighbors = Sync::GetNeighbors( &neighbor_count );
                JsonArray neighbor_array = doc->createNestedArray( "neighbors" );
                for( int i = 0; i < neighbor_count; ++i )
                {
                    neighbor_array.add( neighbors[ i ].mHostName );
                }
            } );
            httpd_resp_set_type( req, "application/json" );
            httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
            httpd_resp_send( req, settings_string.c_str(), HTTPD_RESP_USE_STRLEN );
            return ESP_OK;
        }

        esp_err_t HandleOptions( httpd_req_t* req )
        {
            Serial.println( "set settings options" );
            httpd_resp_set_type( req, "application/json" );
            httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
            httpd_resp_set_hdr( req, "Access-Control-Allow-Headers", "*" );
            httpd_resp_set_hdr( req, "Access-Control-Allow-Methods", "POST, OPTIONS" );
            httpd_resp_set_status( req, "204 No Content" );
            httpd_resp_send( req, nullptr, 0 );
            return ESP_OK;
        }

        esp_err_t HandleResetNeighbors( httpd_req_t* req )
        {
            Serial.println( "Refresh Neighbors" );
            if( QueueDiscoveryCallback )
            {
                QueueDiscoveryCallback();
            }
            /* Send a simple response */
            const char resp[] = "done";
            httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
            httpd_resp_send( req, resp, HTTPD_RESP_USE_STRLEN );
            return ESP_OK;
        }

        esp_err_t HandleResetEsp( httpd_req_t* req )
        {
            Serial.println( "Reset Requested!" );
            ESP.restart();
            /* Send a simple response */
            const char resp[] = "done";
            httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
            httpd_resp_send( req, resp, HTTPD_RESP_USE_STRLEN );
            return ESP_OK;
        }

        esp_err_t HandleSetSettings( httpd_req_t* req )
        {
            PROFILE_FUNC;
            Serial.println( "set settings" );
            char content[ 1024 ];

            if( req->content_len >= sizeof( content ) ) // ensure room for null terminator.
            {
                Serial.println( "request too large!" );
                Serial.println( req->content_len );
                return ESP_FAIL;
            }

            bool isBroadcast = httpd_req_get_hdr_value_len( req, BroadcastHeaderName ) > 0;

            int ret = httpd_req_recv( req, content, req->content_len );
            if( ret <= 0 )
            { /* 0 return value indicates connection closed */
                /* Check if timeout occurred */
                if( ret == HTTPD_SOCK_ERR_TIMEOUT )
                {
                    /* In case of timeout one can choose to retry calling
                     * httpd_req_recv(), but to keep it simple, here we
                     * respond with an HTTP 408 (Request Timeout) error */
                    httpd_resp_send_408( req );
                }
                /* In case of error, returning ESP_FAIL will
                 * ensure that the underlying socket is closed */
                return ESP_FAIL;
            }
            content[ req->content_len ] = 0; // add null terminator.
            Serial.println( content );

            if( Settings::DeserializePartialSettings( Settings::Settings, content, req->content_len ) )
            {
                Serial.println( "settings set successfully" );
                if( BroadcastSettingsCallback && !isBroadcast )
                {
                    BroadcastSettingsCallback();
                }
            }
            else
            {
                Serial.println( "failed to set settings" );
            }

            if( !Settings::SaveToFlash( Settings::Settings ) )
            {
                Serial.println( "failed to save to flash" );
            }

            /* Send a simple response */
            const char resp[] = "Settings Accepted!";
            httpd_resp_set_hdr( req, "Access-Control-Allow-Origin", "*" );
            httpd_resp_send( req, resp, HTTPD_RESP_USE_STRLEN );
            return ESP_OK;
        }

        httpd_uri_t GetHomepage = { .uri = "/", .method = HTTP_GET, .handler = HandleGetHomepage, .user_ctx = NULL };
        httpd_uri_t GetSettings = { .uri = "/settings", .method = HTTP_GET, .handler = HandleGetSettings, .user_ctx = NULL };

        httpd_uri_t SetSettingsOptions = { .uri = "/*", .method = HTTP_OPTIONS, .handler = HandleOptions, .user_ctx = NULL };

        httpd_uri_t SetSettings = { .uri = "/setSettings", .method = HTTP_POST, .handler = HandleSetSettings, .user_ctx = NULL };
        httpd_uri_t ResetNeighbors = { .uri = "/resetNeighbors", .method = HTTP_POST, .handler = HandleResetNeighbors, .user_ctx = NULL };
        httpd_uri_t ResetEsp = { .uri = "/resetEsp", .method = HTTP_POST, .handler = HandleResetEsp, .user_ctx = NULL };

    }


    void Start( std::function<void()> queue_broadcast, std::function<void()> queue_discovery )
    {
        BroadcastSettingsCallback = queue_broadcast;
        QueueDiscoveryCallback = queue_discovery;
        /* Generate default configuration */
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();

        /* Empty handle to esp_http_server */
        ServerHandle = NULL;

        /* Start the httpd server */
        if( httpd_start( &ServerHandle, &config ) == ESP_OK )
        {
            /* Register URI handlers */
            httpd_register_uri_handler( ServerHandle, &GetHomepage );
            httpd_register_uri_handler( ServerHandle, &GetSettings );

            httpd_register_uri_handler( ServerHandle, &SetSettingsOptions );

            httpd_register_uri_handler( ServerHandle, &SetSettings );
            httpd_register_uri_handler( ServerHandle, &ResetNeighbors );
            httpd_register_uri_handler( ServerHandle, &ResetEsp );
        }
        else
        {
            Serial.println( "failed to start web server" );
        }
    }
    void Stop()
    {
        if( ServerHandle )
        {
            /* Stop the httpd server */
            httpd_stop( ServerHandle );
        }
    }
}