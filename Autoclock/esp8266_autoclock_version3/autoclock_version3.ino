#include <ESP8266WiFi.h>        // Biblioteca para Wifi
#include <ESP8266WebServer.h>   // Bibliteca Para Servidor
#include <LiquidCrystal.h>      // Biblioteca para o Display
#include <NTPClient.h>          // Biblioteca para NTP
#include <WiFiUdp.h>            // Biblioteca para uso de UDP
#include <Wire.h>               // Biblioteca I2C
#include <RTClib.h>             // Biblioteca do Real Time CLock
#include <FS.h>                 // Sistema de Arquivos
#include <ArduinoJson.h>        // Interpretação de Arquivos JSON
#include <ESP8266HTTPClient.h>  // Cliente Http
#include <ESP8266Ping.h>        // Biblioteca para usar ping

// Nomeclatura da Máquina de Estados
#define wifiConnectState 1
#define stacionModeState 2
#define accessPointModeState 3
#define getOffsetState 4
#define NTPReadState 5
#define setRTCState 6
#define getRTCState 7
#define displayTimeState 8
#define idle1SecState 9
#define stopState 10


// Variáveis de Estado
int currentState = wifiConnectState;
int nextState = wifiConnectState;
bool firstGetOffset = true;
bool firstNTPRead = true;
bool rtcSeted = false;
bool manualAjustRtcSeted = false;   // Usada para que não haja reload dos dados do horário para ajuste manual


// Váriaveis da Rede que Será Gerada Pelo ESP8266
ESP8266WebServer server;
char* clockPassword = "";
char* clockSsid = "AutoClock";


// Informações da Rede Gerada Pelo ESP8266
IPAddress local_ip(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress netmask(255,255,255,0);


// WebPage
char webpage[] PROGMEM = R"=====(
<span style="display: none;">"</span>
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Clock Setup</title>
        <style>
            * {
                padding: 0;
                margin: 0;
            }

            .title {
                grid-area: cabecalho;
            }

            body {
                grid-area: conteudo;
            }

            .blank-area1 {
                padding: 5px;
            }

            .clock {
                grid-area: clock;
            }

            footer {
                grid-area: rodape;
                margin: 0;
                padding: 0;
            }

            body {
                display: grid;
                min-height: 100vh;
                grid-template-columns: 1fr;
                grid-template-rows: 100px 1fr 30px;
                grid-template-areas: 
                    'cabecalho'
                    'conteudo'
                    'blank-area1'
                    'clock'
                    'blank-area2'
                    'rodape';
            }
            html {
                margin: 0;
                padding: 0;
            }

            body {
                margin: 0;
                padding: 0;
                background-color: lightgrey;
            }

            form {
                margin: 0;
                padding: 0;
            }

            fieldset {
                margin: 0;
                padding: 0;
                background-color: lightgrey;
                border: medium 5px darkslategrey;
            }

            .title {
                padding: 0;
                background-color: darkblue;
                color: yellow;
                height: 75px;
                border: solid 5px darkgoldenrod;
            }

            .title h1 {
                display: flex;
                justify-content: center;
                margin-top: 15px;
            }

            .buttons {
                display: flex;
                justify-content: center;
            }

            .espaco {
                float: left;
                padding: 20px;
            }

            .ssid {
                margin-left: 56px;
            }

            .date-type {
                padding: 10px 0px;
            }

            .dateTimeInfo {
                margin-bottom: 100px;
                display: flex;
                flex-direction: column;
                align-items: center;
                /* justify-content: flex-start; */
            }

            .time {
                background-color: blue;
                color: yellow;
                font-family: sans-serif;
                font-size: 30px;
                border: double 5px black;
                padding: 5px;
            }

            .date {
                font-size: 35px;
                padding: 5px; 
                text-decoration: underline;               
            }

            .espaco {
                float: left;
                padding: 20px;
            }

        </style>
    </head>
    <body>
        <div class="title">
            <h1>Clock Setup</h1>
        </div>
        <form>
            <fieldset>
                <fieldset>
                    <div>
                        <label for="ssid">SSID</label>      
                        <input value="" id="ssid" placeholder="SSID" class="ssid">
                    </div>
                    <div>
                        <label for="password">PASSWORD</label>
                        <input type="password" value="" id="password" placeholder="PASSWORD">
                    </div>
                </fieldset>
                <fieldset>
                    <div>
                        <label for="time-format-display">Display</label>
                        <input type="radio" name="time-format" value="12"> 12h
                        <input type="radio" name="time-format" value="24"> 24h
                    </div>
                    </fieldset>
                    <fieldset>
                    <div>
                    <label for="country">Country</label>
                    <br>
                        <select id="Country" name="Country">
                            <option value="UTC">No Time Zone</option>
                            <option value="AF">Afghanistan</option>
                            <option value="AL">Albania</option>
                            <option value="DZ">Algeria</option>
                            <option value="AS">American Samoa</option>
                            <option value="AD">Andorra</option>
                            <option value="AO">Angola</option>
                            <option value="AI">Anguilla</option>
                            <option value="AQ">Antarctica</option>
                            <option value="AG">Antigua and Barbuda</option>
                            <option value="AR">Argentina</option>
                            <option value="AM">Armenia</option>
                            <option value="AW">Aruba</option>
                            <option value="AU">Australia</option>
                            <option value="AT">Austria</option>
                            <option value="AZ">Azerbaijan</option>
                            <option value="BS">Bahamas</option>
                            <option value="BH">Bahrain</option>
                            <option value="BD">Bangladesh</option>
                            <option value="BB">Barbados</option>
                            <option value="BY">Belarus</option>
                            <option value="BE">Belgium</option>
                            <option value="BZ">Belize</option>
                            <option value="BJ">Benin</option>
                            <option value="BM">Bermuda</option>
                            <option value="BT">Bhutan</option>
                            <option value="BO">Bolivia, Plurinational State of</option>
                            <option value="BQ">Bonaire, Sint Eustatius and Saba</option>
                            <option value="BA">Bosnia and Hezergovina</option>
                            <option value="BW">Botswana</option>
                            <option value="BR">Brazil</option>
                            <option value="IO">British Indian Ocean Territory</option>
                            <option value="BN">Brunei Darussalam</option>
                            <option value="BG">Bulgaria</option>
                            <option value="BF">Burkina Faso</option>
                            <option value="BI">Burundi</option>
                            <option value="KH">Cambodia</option>
                            <option value="CM">Cameroon</option>
                            <option value="CA">Canada</option>
                            <option value="CV">Cape Verde</option>
                            <option value="KY">Cayman Island</option>
                            <option value="CF">Central African Republic</option>
                            <option value="TD">Chad</option>
                            <option value="CL">Chile</option>
                            <option value="CN">China</option>
                            <option value="CX">Christmas Island</option>
                            <option value="CC">Cocos (Keeling) Islands</option>
                            <option value="CO">Colombia</option>
                            <option value="KM">Comoros</option>
                            <option value="CG">Congo</option>
                            <option value="CD">Congo, the Democratic Republic of the</option>
                            <option value="CK">Cook Islands</option>
                            <option value="CR">Costa Rica</option>
                            <option value="HR">Croatia</option>
                            <option value="CU">Cuba</option>
                            <option value="CW">Curaçao</option>
                            <option value="CY">Cyprus</option>
                            <option value="CZ">Czech Republic</option>
                            <option value="CI">Côte d'lvoire</option>
                            <option value="DK">Denmark</option>
                            <option value="DJ">Djibouti</option>
                            <option value="DM">Dominica</option>
                            <option value="DO">Dominican Republic</option>
                            <option value="EC">Ecuador</option>
                            <option value="EG">Egypt</option>
                            <option value="SV">El Salvador</option>
                            <option value="GQ">Equatorial Guinea</option>
                            <option value="ER">Eritrea</option>
                            <option value="EE">Estonia</option>
                            <option value="ET">Ethiopia</option>
                            <option value="FK">Falkland Islands (Malvinas)</option>
                            <option value="FO">Faroe Islands</option>
                            <option value="FJ">Fiji</option>
                            <option value="FI">Finland</option>
                            <option value="FR">France</option>
                            <option value="GF">French Guiana</option>
                            <option value="PF">French Polynesia</option>
                            <option value="TF">French Southern Territories</option>
                            <option value="GA">Gabon</option>
                            <option value="GM">Gambia</option>
                            <option value="GE">Georgia</option>
                            <option value="DE">Germany</option>
                            <option value="GH">Ghana</option>
                            <option value="GI">Gibraltar</option>
                            <option value="GR">Greece</option>
                            <option value="GL">Greenland</option>
                            <option value="GD">Grenada</option>
                            <option value="GP">Guadeloupe</option>
                            <option value="GU">Guam</option>
                            <option value="GT">Guatemala</option>
                            <option value="GG">Guernsey</option>
                            <option value="GN">Guinea</option>
                            <option value="GW">Guine-Bissau</option>
                            <option value="GY">Guyana</option>
                            <option value="HT">Haiti</option>
                            <option value="VA">Holy See (Vatican City State)</option>
                            <option value="HN">Honduras</option>
                            <option value="HK">Hong Kong</option>
                            <option value="HU">Hungary</option>
                            <option value="IS">Iceland</option>
                            <option value="IN">India</option>
                            <option value="ID">Indonesia</option>
                            <option value="IR">Iran, Islamic Republic of</option>
                            <option value="IQ">Iraq</option>
                            <option value="IE">Ireland</option>
                            <option value="IL">Israel</option>
                            <option value="IT">Italy</option>
                            <option value="JM">Jamaica</option>
                            <option value="JP">Japan</option>
                            <option value="JE">Jersey</option>
                            <option value="JO">Jordan</option>
                            <option value="KZ">Kazakhstan</option>
                            <option value="KE">Kenya</option>
                            <option value="KI">Kiribati</option>
                            <option value="KP">Korea, Democratic People's Republic of</option>
                            <option value="KR">Korea, Republic of</option>
                            <option value="KW">Kuwait</option>
                            <option value="KG">Kyrgyzstan</option>
                            <option value="LA">Lao People's Democratic Republic</option>
                            <option value="LV">Latvia</option>
                            <option value="LB">Lebanon</option>
                            <option value="LS">Lesotho</option>
                            <option value="LR">Liberia</option>
                            <option value="LY">Libya</option>
                            <option value="LI">Liechtenstein</option>
                            <option value="LT">Lithuania</option>
                            <option value="LU">Luxembourg</option>
                            <option value="MO">Macao</option>
                            <option value="MK">Macedonia, the Former Yugoslav Republic of</option>
                            <option value="MG">Madagascar</option>
                            <option value="MW">Malawi</option>
                            <option value="MY">Malaysia</option>
                            <option value="MV">Maldives</option>
                            <option value="ML">Mali</option>
                            <option value="MT">Malta</option>
                            <option value="MH">Marshall Islands</option>
                            <option value="MQ">Martinique</option>
                            <option value="MR">Mauritania</option>
                            <option value="MU">Mauritius</option>
                            <option value="YT">Mayotte</option>
                            <option value="MX">Mexico</option>
                            <option value="FM">Micronesia, Federated State of</option>
                            <option value="MD">Moldova, Republic of</option>
                            <option value="MC">Monaco</option>
                            <option value="MN">Mongolia</option>
                            <option value="ME">Montenegro</option>
                            <option value="MS">Montserrat</option>
                            <option value="MA">Morocco</option>
                            <option value="MZ">Mozambique</option>
                            <option value="MM">Myanmar</option>
                            <option value="NA">Namibia</option>
                            <option value="NR">Nauru</option>
                            <option value="NP">Nepal</option>
                            <option value="NL">Netherlands</option>
                            <option value="NC">New Caledonia</option>
                            <option value="NZ">New Zeland</option>
                            <option value="NI">Nicaragua</option>
                            <option value="NE">Niger</option>
                            <option value="NG">Nigeria</option>
                            <option value="NU">Niue</option>
                            <option value="NF">Norfolk Island</option>
                            <option value="MP">Nothern Mariana Islands</option>
                            <option value="NO">Norway</option>
                            <option value="OM">Oman</option>
                            <option value="PK">Pakistan</option>
                            <option value="PW">Palau</option>
                            <option value="PS">Palestine, State of</option>
                            <option value="PA">Panama</option>
                            <option value="PG">Papua New Guinea</option>
                            <option value="PY">Paraguay</option>
                            <option value="PE">Peru</option>
                            <option value="PH">Philippines</option>
                            <option value="PN">Pitcairn</option>
                            <option value="PL">Poland</option>
                            <option value="PT">Portugal</option>
                            <option value="PR">Puerto Rico</option>
                            <option value="QA">Qatar</option>
                            <option value="RO">Romania</option>
                            <option value="RU">Russian Federation</option>
                            <option value="RW">Rwanda</option>
                            <option value="RE">Réunion</option>
                            <option value="BL">Saint Barthélemy</option>
                            <option value="SH">Saint Helena, Ascension and Tristan da Cunha</option>
                            <option value="KN">Saint Kitts and Nevis</option>
                            <option value="LC">Saint Lucia</option>
                            <option value="MF">Saint Martin (French part)</option>
                            <option value="PM">Saint Pierre and Miquelon</option>
                            <option value="VC">Saint Vincent and the Grenadines</option>
                            <option value="WS">Samoa</option>
                            <option value="SM">San Marino</option>
                            <option value="ST">Sao Tome and Principe</option>
                            <option value="SA">Saudi Arabia</option>
                            <option value="SN">Senegal</option>
                            <option value="RS">Serbia</option>
                            <option value="SC">Seychelles</option>
                            <option value="SL">Sierra Leone</option>
                            <option value="SG">Singapore</option>
                            <option value="SX">Sint Maarten (Dutch part)</option>
                            <option value="SK">Slovakia</option>
                            <option value="SI">Slovenia</option>
                            <option value="SB">Solomon Islands</option>
                            <option value="SO">Somalia</option>
                            <option value="ZA">South Africa</option>
                            <option value="GS">South Georgia and the South Sandwich Islands</option>
                            <option value="SS">South Sudan</option>
                            <option value="ES">Spain</option>
                            <option value="LK">Sri Lanka</option>
                            <option value="SD">Sudan</option>
                            <option value="SR">Suriname</option>
                            <option value="SJ">Svalbard and Jan Mayen</option>
                            <option value="SZ">Swaziland</option>
                            <option value="SE">Sweden</option>
                            <option value="CH">Switzerland</option>
                            <option value="SY">Syrian Arab Republic</option>
                            <option value="TW">Taiwan, Province of China</option>
                            <option value="TJ">Tajikistan</option>
                            <option value="TH">Thailand</option>
                            <option value="TL">Timor-Leste</option>
                            <option value="TG">Togo</option>
                            <option value="TK">Tokelau</option>
                            <option value="TO">Tonga</option>
                            <option value="TT">Trinidad and Tobago</option>
                            <option value="TN">Tunisia</option>
                            <option value="TR">Turkey</option>
                            <option value="TM">Turkmenistan</option>
                            <option value="TC">Turks and Caiscos Islands</option>
                            <option value="TV">Tuvalu</option>
                            <option value="UG">Uganda</option>
                            <option value="UA">Ukraine</option>
                            <option value="AE">United Arab Emirates</option>
                            <option value="GB">United Kingdom</option>
                            <option value="US">United States</option>
                            <option value="UM">United States Minor Outlying Islands</option>
                            <option value="UY">Uruguay</option>
                            <option value="UZ">Uzbekistan</option>
                            <option value="VU">Vanuatu</option>
                            <option value="VE">Venezuela, Bolivarian Republic of</option>
                            <option value="VN">Viet Nam</option>
                            <option value="VG">Virgin Islands, British</option>
                            <option value="VI">Virgin Islands, U.S.</option>
                            <option value="WF">Wallis and Futuna</option>
                            <option value="EH">Western Sahara</option>
                            <option value="YE">Yemen</option>
                            <option value="ZM">Zambia</option>
                            <option value="ZW">Zimbabwe</option>
                            <option value="AX">Åland Islands</option>
                        </select>
                        <br>
                        <label for="tz">Time Zone</label><br>
                        <span id="tzs">

                            <select name="noTimeZone" id="UTC" style="display: none;" selectable>
                                <option value="Etc/GMT">Coordinated Universal Time</option>
                            </select>

                            <select name="Afghanistan" id="AF" style="display: none;" selectable>
                                <option value="Asia/Kabul">Kabul</option>
                            </select>

                            <select name="Albania" id="AL" style="display: none;" selectable>
                                <option value="Europe/Tirane">Tirane</option>
                            </select>

                            <select name="Algeria" id="DZ" style="display: none;" selectable>
                                <option value="Africa/Algiers">Algiers</option>
                            </select>

                            <select name="American Samoa" id="AS" style="display: none;" selectable>
                                <option value="Pacific/Pago_Pago">Pago Pago</option>
                            </select>

                            <select name="Andorra" id="AD" style="display: none;" selectable>
                                <option value="Europe/Andorra">Andorra</option>
                            </select>

                            <select name="Angola" id="AO" style="display: none;" selectable>
                                <option value="Africa/Luanda">Luanda</option>
                            </select>

                            <select name="Anguilla" id="AI" style="display: none;" selectable>
                                <option value="America/Anguilla">Anguilla</option>
                            </select>

                            <select name="Antarctica" id="AQ" style="display: none;" selectable>
                                <option value="Antarctica/Casey">Casey</option>
                                <option value="Antarctica/Davis">Davis</option>
                                <option value="Antarctica/DumontDUrville">Dumont D'Urville</option>
                                <option value="Antarctica/Mawson">Mawson</option>
                                <option value="Antarctica/Palmer">Palmer</option>
                                <option value="Antarctica/Rothera">Rothera</option>
                                <option value="Antarctica/Syowa">Syowa</option>
                                <option value="Antarctica/Troll">Troll</option>
                                <option value="Antarctica/Vostok">Vostok</option>
                            </select>

                            <select name="Antigua and Barbuda" id="AG" style="display: none;" selectable>
                                <option value="America/Antigua">Antigua</option>
                            </select>

                            <select name="Argentina" id="AR" style="display: none;" selectable>
                                <option value="America/Argentina/Buenos_Aires">Buenos Aires</option>
                                <option value="America/Argentina/Catamarca">Catamarca</option>
                                <option value="America/Argentina/Cordoba">Cordoba</option>
                                <option value="America/Argentina/Jujuy">Jujuy</option>
                                <option value="America/Argentina/La_Rioja">La Rioja</option>
                                <option value="America/Argentina/Mendoza">Mendoza</option>
                                <option value="America/Argentina/Rio_Gallegos">Rio Gallegos</option>
                                <option value="America/Argentina/Salta">Salta</option>
                                <option value="America/Argentina/San_Juan">San Juan</option>
                                <option value="America/Argentina/San_Luis">San Luis</option>
                                <option value="America/Argentina/Tucuman">Tucuman</option>
                                <option value="America/Argentina/Ushuaia">Ushuaia</option>
                            </select>

                            <select name="Armenia" id="AM" style="display: none;" selectable>
                                <option value="Asia/Yerevan">Yerevan</option>
                            </select>

                            <select name="Aruba" id="AW" style="display: none;" selectable>
                                <option value="America/Aruba">Aruba</option>
                            </select>

                            <select name="Australia" id="AU" style="display: none;" selectable>
                                <option value="Antarctica/Macquarie">Macquarie</option>
                                <option value="Australia/Adelaide">Adelaide</option>
                                <option value="Australia/Brisbane">Brisbane</option>
                                <option value="Australia/Broken_Hill">Broken Hill</option>
                                <option value="Australia/Currie">Currie</option>
                                <option value="Australia/Darwin">Darwin</option>
                                <option value="Australia/Eucla">Eucla</option>
                                <option value="Australia/Hobart">Hobart</option>
                                <option value="Australia/Lindeman">Lindeman</option>
                                <option value="Australia/Lord_Howe">Lord Howe</option>
                                <option value="Australia/Melbourne">Melbourne</option>
                                <option value="Australia/Perth">Perth</option>
                                <option value="Australia/Sydney">Sydney</option>
                            </select>

                            <select name="Austria" id="AT" style="display: none;" selectable>
                                <option value="Europe/Vienna">Vienna</option>
                            </select>

                            <select name="Azerbaijan" id="AZ" style="display: none;" selectable>
                                <option value="Asia/Baku">Baku</option>
                            </select>
                            
                            <select name="Bahamas" id="BS" style="display: none;" selectable>
                                <option value="America/Nassau">Nassau</option>
                            </select>

                            <select name="Bahrain" id="BH" style="display: none;" selectable>
                                <option value="Asia/Bahrain">Bahrain</option>
                            </select>

                            <select name="Bangladesh" id="BD" style="display: none;" selectable>
                                <option value="Asia/Dhaka">Dhaka</option>
                            </select>

                            <select name="Barbados" id="BB" style="display: none;" selectable>
                                <option value="America/Barbados">Barbados</option>
                            </select>

                            <select name="Belarus" id="BY" style="display: none;" selectable>
                                <option value="Europe/Minsk">Minsk</option>
                            </select>

                            <select name="Belgium" id="BE" style="display: none;" selectable>
                                <option value="Europe/Brussels">Brussels</option>
                            </select>

                            <select name="Belize" id="BZ" style="display: none;" selectable>
                                <option value="America/Belize">Belize</option>
                            </select>

                            <select name="Benin" id="BJ" style="display: none;" selectable>
                                <option value="Africa/Porto-Novo">Porto Novo</option>
                            </select>

                            <select name="Bermuda" id="BM" style="display: none;" selectable>
                                <option value="Atlantic/Bermuda">Bermuda</option>
                            </select>

                            <select name="Bhutan" id="BT" style="display: none;" selectable>
                                <option value="Asia/Thimphu">Thimphu</option>
                            </select>

                            <select name="Bolivia, Plurinational State of" id="BO" style="display: none;" selectable>
                                <option value="America/La_Paz">La Paz</option>
                            </select>

                            <select name="Bonaire" id="BQ" style="display: none;" selectable>
                                <option value="America/Kralendijk">Kralendijk</option>
                            </select>

                            <select name="Bosnia" id="BA" style="display: none;" selectable>
                                <option value="Europe/Sarajevo">Sarajevo</option>
                            </select>

                            <select name="Botswana" id="BW" style="display: none;" selectable>
                                <option value="Africa/Gaborone">Gaborone</option>
                            </select>

                            <select name="Brazil" id="BR" style="display: none;" selectable>
                                <option value="America/Araguaina">Araguaina</option>
                                <option value="America/Bahia">Bahia</option>
                                <option value="America/Belem">Belem</option>
                                <option value="America/Boa_Vista">Boa Vista</option>
                                <option value="America/Campo_Grande">Campo Grande</option>
                                <option value="America/Cuiaba">Cuiaba</option>
                                <option value="America/Eirunepe">Eirunepe</option>
                                <option value="America/Fortaleza">Fortaleza</option>
                                <option value="America/Maceio">Maceio</option>
                                <option value="America/Manaus">Manaus</option>
                                <option value="America/Noronha">Noronha</option>
                                <option value="America/Porto_Velho">Porto Velho</option>
                                <option value="America/Recife">Recife</option>
                                <option value="America/Rio_Branco">Rio Branco</option>
                                <option value="America/Santarem">Santarem</option>
                                <option value="America/Sao_Paulo">Sao Paulo</option>
                            </select>

                            <select name="British Indian Ocean Territory" id="IO" style="display: none;" selectable>
                                <option value="Indian/Chagos">Chagos</option>
                            </select>

                            <select name="Brunei Darussalam" id="BN" style="display: none;" selectable>
                                <option value="Asia/Brunei">Brunei</option>
                            </select>

                            <select name="Bulgaria" id="BG" style="display: none;" selectable>
                                <option value="Europe/Sofia">Sofia</option>
                            </select>

                            <select name="Burkina Faso" id="BF" style="display: none;" selectable>
                                <option value="Africa/Ouagadougou">Ouagadougou</option>
                            </select>

                            <select name="Burundi" id="BI" style="display: none;" selectable>
                                <option value="Africa/Bujumbura">Bujumbura</option>
                            </select>

                            <select name="Cambodia" id="KH" style="display: none;" selectable>
                                <option value="Asia/Phnom_Penh">Phnom Penh</option>
                            </select>

                            <select name="Cameroon" id="CM" style="display: none;" selectable>
                                <option value="Africa/Douala">Douala</option>
                            </select>

                            <select name="Canada" id="CA" style="display: none;" selectable>
                                <option value="America/Atikokan">Atikokan</option>
                                <option value="America/Blanc-Sablon">Blanc Sablon</option>
                                <option value="America/Cambridge_Bay">Cambridge Bay</option>
                                <option value="America/Creston">Creston</option>
                                <option value="America/Dawson">Dawson</option>
                                <option value="America/Dawson_Creek">Dawson Creek</option>
                                <option value="America/Edmonton">Edmonton</option>
                                <option value="America/Fort_Nelson">Fort Nelson</option>
                                <option value="America/Glace_Bay">Glace Bay</option>
                                <option value="America/Goose_Bay">Goose Bay</option>
                                <option value="America/Halifax">Halifax</option>
                                <option value="America/Inuvik">Inuvik</option>
                                <option value="America/Iqaluit">Iqaluit</option>
                                <option value="America/Moncton">Moncton</option>
                                <option value="America/Nipigon">Nipigon</option>
                                <option value="America/Pangnirtung">Pangnirtung</option>
                                <option value="America/Rainy_River">Rainy River</option>
                                <option value="America/Rankin_Inlet">Rankin Inlet</option>
                                <option value="America/Regina">Regina</option>
                                <option value="America/Resolute">Resolute</option>
                                <option value="America/St_Johns">St Johns</option>
                                <option value="America/Swift_Current">Swift Current</option>
                                <option value="America/Thunder_Bay">Thunder Bay</option>
                                <option value="America/Toronto">Toronto</option>
                                <option value="America/Vancouver">Vancouver</option>
                                <option value="America/Whitehorse">Whitehorse</option>
                                <option value="America/Winnipeg">Winnipeg</option>
                                <option value="America/Yellowknife">Yellowknife</option>
                            </select>

                            <select name="Cape Verde" id="CV" style="display: none;" selectable>
                                <option value="Atlantic/Cape_Verde">Cape Verde</option>
                            </select>

                            <select name="Cayman Island" id="KY" style="display: none;" selectable>
                                <option value="America/Cayman">Cayman</option>
                            </select>

                            <select name="Central African Republic" id="CF" style="display: none;" selectable>
                                <option value="Africa/Bangui">Bangui</option>
                            </select>

                            <select name="Chad" id="TD" style="display: none;" selectable>
                                <option value="Africa/Ndjamena">Ndjamena</option>
                            </select>

                            <select name="Chile" id="CL" style="display: none;" selectable>
                                <option value="America/Punta_Arenas">Punta Arenas</option>
                                <option value="America/Santiago">San Tiago</option>
                                <option value="Pacific/Easter">Easter</option>
                            </select>

                            <select name="China" id="CN" style="display: none;" selectable>
                                <option value="Asia/Shanghai">Shanghai</option>
                                <option value="Asia/Urumqi">Urumqi</option>
                            </select>

                            <select name="Christmas Island" id="CX" style="display: none;" selectable>
                                <option value="Indian/Christmas">Christmas</option>
                            </select>

                            <select name="Cocos (Keeling) Islands" id="CC" style="display: none;" selectable>
                                <option value="Indian/Cocos">Cocos</option>
                            </select>

                            <select name="Colombia" id="CO" style="display: none;" selectable>
                                <option value="America/Bogota">Bogota</option>
                            </select>

                            <select name="Comoros" id="KM" style="display: none;" selectable>
                                <option value="Indian/Comoro">Comoro</option>
                            </select>

                            <select name="Congo" id="CG" style="display: none;" selectable>
                                <option value="Africa/Brazzaville">Brazzaville</option>
                            </select>

                            <select name="Congo, the Democratic Republic of the" id="CD" style="display: none;" selectable>
                                <option value="Africa/Kinshasa">Kinshasa</option>
                                <option value="Africa/Lubumbashi">Lubumbashi</option>
                            </select>

                            <select name="Cook Island" id="CK" style="display: none;" selectable>
                                <option value="Pacific/Rarotonga">Rarotonga</option>
                            </select>

                            <select name="Costa Rica" id="CR" style="display: none;" selectable>
                                <option value="America/Costa_Rica">Costa Rica</option>
                            </select>
                            
                            <select name="Croatia" id="CR" style="display: none;" selectable>
                                <option value="Europe/Zagreb">Zagreb</option>
                            </select>

                            <select name="Cuba" id="CU" style="display: none;" selectable>
                                <option value="America/Havana">Havana</option>
                            </select>

                            <select name="Curaçao" id="CW" style="display: none;" selectable>
                                <option value="America/Curacao">Curaçao</option>
                            </select>

                            <select name="Cyprus" id="CY" style="display: none;" selectable>
                                <option value="Asia/Famagusta">Famagusta</option>
                                <option value="Asia/Nicosia">Nicosia</option>
                            </select>

                            <select name="Czech Republic" id="CZ" style="display: none;" selectable>
                                <option value="Europe/Prague">Prague</option>
                            </select>

                            <select name="Cote d'lvoire" id="CI" style="display: none;" selectable>
                                <option value="Africa/Abidjan">Abidjan</option>
                            </select>

                            <select name="Denmark" id="DK" style="display: none;" selectable>
                                <option value="Europe/Copenhagen">Copenhagen</option>
                            </select>

                            <select name="Djibouti" id="DJ" style="display: none;" selectable>
                                <option value="Africa/Djibouti">Djibouti</option>
                            </select>

                            <select name="Dominica" id="DM" style="display: none;" selectable>
                                <option value="America/Dominica">Dominica</option>
                            </select>

                            <select name="Dominican Republic" id="DO" style="display: none;" selectable>
                                <option value="America/Santo_Domingo">Santo Domingo</option>
                            </select>

                            <select name="Ecuador" id="EC" style="display: none;" selectable>
                                <option value="America/Guayaquil">Guayaquil</option>
                                <option value="Pacific/Galapagos">Galapagos</option>
                            </select>

                            <select name="Egypt" id="EG" style="display: none;" selectable>
                                <option value="Africa/Cairo">Cairo</option>
                            </select>

                            <select name="El Salvador" id="SV" style="display: none;" selectable>
                                <option value="America/El_Salvador">El Salvador</option>
                            </select>

                            <select name="Equatorial Guinea" id="GQ" style="display: none;" selectable>
                                <option value="Africa/Malabo">Malabo</option>
                            </select>

                            <select name="Eritrea" id="ER" style="display: none;" selectable>
                                <option value="Africa/Asmara">Asmara</option>
                            </select>

                            <select name="Estonia" id="EE" style="display: none;" selectable>
                                <option value="Europe/Tallinn">Tallinn</option>
                            </select>

                            <select name="Ethiopia" id="ET" style="display: none;" selectable>
                                <option value="Africa/Addis_Ababa">Addis Ababa</option>
                            </select>

                            <select name="Falkland Islands (Malvinas)" id="FK" style="display: none;" selectable>
                                <option value="Atlantic/Stanley">Stanley</option>
                            </select>

                            <select name="Faroe Islands" id="FO" style="display: none;" selectable>
                                <option value="Atlantic/Faroe">Faroe</option>
                            </select>

                            <select name="Fiji" id="FJ" style="display: none;" selectable>
                                <option value="Pacific/Fiji">Fiji</option>
                            </select>

                            <select name="Finland" id="FI" style="display: none;" selectable>
                                <option value="Europe/Helsinki">Helsinki</option>
                            </select>

                            <select name="France" id="FR" style="display: none;" selectable>
                                <option value="Europe/Paris">Paris</option>
                            </select>

                            <select name="French Guiana" id="GF" style="display: none;" selectable>
                                <option value="America/Cayenne">Cayenne</option>
                            </select>

                            <select name="French Polynesia" id="PF" style="display: none;" selectable>
                                <option value="Pacific/Gambier">Gambier</option>
                                <option value="Pacific/Marquesas">Marquesas</option>
                                <option value="Pacific/Tahiti">Tahiti</option>
                            </select>

                            <select name="French Southern Territories" style="display: none;" id="TF" selectable>
                                <option value="Indian/Kerguelen">Kerguelen</option>
                            </select>

                            <select name="Gabon" id="GA" style="display: none;" selectable>
                                <option value="Africa/Libreville">Libreville</option>
                            </select>

                            <select name="Gambia" id="GM" style="display: none;" selectable>
                                <option value="Africa/Banjul">Banjul</option>
                            </select>

                            <select name="Georgia" id="GE" style="display: none;" selectable>
                                <option value="Asia/Tbilisi">Tbilisi</option>
                            </select>

                            <select name="Germany" id="DE" style="display: none;" selectable>
                                <option value="Europe/Berlin">Berlin</option>
                                <option value="Europe/Busingen">Busingen</option>
                            </select>

                            <select name="Ghana" id="GH" style="display: none;" selectable>
                                <option value="Africa/Accra">Accra</option>
                            </select>

                            <select name="Gibraltar" id="GI" style="display: none;" selectable>
                                <option value="Europe/Gibraltar">Gibraltar</option>
                            </select>

                            <select name="Greece" id="GR" style="display: none;" selectable>
                                <option value="Europe/Athens">Athens</option>
                            </select>

                            <select name="Greenland" id="GL" style="display: none;" selectable>
                                <option value="America/Danmarkshavn">Danmarkshavn</option>
                                <option value="America/Godthab">Godthab</option>
                                <option value="America/Scoresbysund">Scoresbysund</option>
                                <option value="America/Thule">Thule</option>
                            </select>

                            <select name="Grenada" id="GD" style="display: none;" selectable>
                                <option value="America/Grenada">Grenada</option>
                            </select>

                            <select name="Guadeloupe" id="GP" style="display: none;" selectable>
                                <option value="America/Guadeloupe">Guadeloupe</option>
                            </select>

                            <select name="Guam" id="GU" style="display: none;" selectable>
                                <option value="Pacific/Guam">Guam</option>
                            </select>

                            <select name="Guatemala" id="GT" style="display: none;" selectable>
                                <option value="America/Guatemala">Guatemala</option>
                            </select>

                            <select name="Guernsey" id="GG" style="display: none;" selectable>
                                <option value="Europe/Guernsey">Guernsey</option>
                            </select>

                            <select name="Guinea" id="GN" style="display: none;" selectable>
                                <option value="Africa/Conakry">Conakry</option>
                            </select>

                            <select name="Guine-Bissau" id="GW" style="display: none;" selectable>
                                <option value="Africa/Bissau">Bissau</option>
                            </select>

                            <select name="Guyana" id="GY" style="display: none;" selectable>
                                <option value="America/Guyana">Guyana</option>
                            </select>

                            <select name="Haiti" id="HT" style="display: none;" selectable>
                                <option value="America/Port-au-Prince">Port au Prince</option>
                            </select>

                            <select name="Holy See (Vatican City State)" id="VA" style="display: none;" selectable>
                                <option value="Europe/Vatican">Vatican</option>
                            </select>

                            <select name="Honduras" id="HN" style="display: none;" selectable>
                                <option value="America/Tegucigalpa">Tegucigalpa</option>
                            </select>

                            <select name="Hong Kong" id="HK" style="display: none;" selectable>
                                <option value="Asia/Hong_Kong">Hong Kong</option>
                            </select>

                            <select name="Hungary" id="HU" style="display: none;" selectable>
                                <option value="Europe/Budapest">Budapest</option>
                            </select>

                            <select name="Iceland" id="IS" style="display: none;" selectable>
                                <option value="Atlantic/Reykjavik">Reykjavik</option>
                            </select>

                            <select name="India" id="IN" style="display: none;" selectable>
                                <option value="Asia/Kolkata">Kolkata</option>
                            </select>

                            <select name="Indonesia" id="ID" style="display: none;" selectable>
                                <option value="Asia/Jakarta">Jakarta</option>
                                <option value="Asia/Jayapura">Jayapura</option>
                                <option value="Asia/Makassar">Makassar</option>
                                <option value="Asia/Pontianak">Pontianak</option>
                            </select>

                            <select name="Iran, Islamic Republic of" id="IR" style="display: none;" selectable>
                                <option value="Asia/Tehran">Tehran</option>
                            </select>

                            <select name="Iraq" id="IQ" style="display: none;" selectable>
                                <option value="Asia/Baghdad">Baghdad</option>
                            </select>

                            <select name="Ireland" id="IE" style="display: none;" selectable>
                                <option value="Europe/Dublin">Dublin</option>
                            </select>

                            <select name="Israel" id="IL" style="display: none;" selectable>
                                <option value="Asia/Jerusalem">Jerusalem</option>
                            </select>

                            <select name="Italy" id="IT" style="display: none;" selectable>
                                <option value="Europe/Rome">Rome</option>
                            </select>

                            <select name="Jamaica" id="JM" style="display: none;" selectable>
                                <option value="America/Jamaica">Jamaica</option>
                            </select>

                            <select name="Japan" id="JP" style="display: none;" selectable>
                                <option value="Asia/Tokyo">Tokyo</option>
                            </select>

                            <select name="Jersey" id="JE" style="display: none;" selectable>
                                <option value="Europe/Jersey">Jersey</option>
                            </select>

                            <select name="Jordan" id="JO" style="display: none;" selectable>
                                <option value="Asia/Amman">Amman</option>
                            </select>

                            <select name="Kazakhstan" id="KZ" style="display: none;" selectable>
                                <option value="Asia/Almaty">Almaty</option>
                                <option value="Asia/Aqtau">Aqtau</option>
                                <option value="Asia/Aqtobe">Aqtobe</option>
                                <option value="Asia/Atyrau">Atyrau</option>
                                <option value="Asia/Oral">Oral</option>
                                <option value="Asia/Qostanay">Qostanay</option>
                                <option value="Asia/Qyzylorda">Qyzylorda</option>
                            </select>

                            <select name="Kenya" id="KE" style="display: none;" selectable>
                                <option value="Africa/Nairobi">Nairobi</option>
                            </select>

                            <select name="Kiribati" id="KI" style="display: none;" selectable>
                                <option value="Pacific/Enderbury">Enderbury</option>
                                <option value="Pacific/Kiritimati">Kiritimati</option>
                                <option value="Pacific/Tarawa">Tarawa</option>
                            </select>

                            <select name="Korea Democratic People's Republic of" id="KP" style="display: none;" selectable>
                                <option value="Asia/Pyongyang">Pyongyang</option>
                            </select>

                            <select name="Korea, Republic of" id="KR" style="display: none;" selectable>
                                <option value="Asia/Seoul">Seoul</option>
                            </select>

                            <select name="Kuwait" id="KW" style="display: none;" selectable>
                                <option value="Asia/Kuwait">Kuwait</option>
                            </select>

                            <select name="Kyrgyzstan" id="KG" style="display: none;" selectable>
                                <option value="Asia/Bishkek">Bishkek</option>
                            </select>

                            <select name="Lao People's Democratic Republic" id="LA" style="display: none;" selectable>
                                <option value="Asia/Vientiane">Vientiane</option>
                            </select>

                            <select name="Latvia" id="LV" style="display: none;" selectable>
                                <option value="Europe/Riga">Riga</option>
                            </select>

                            <select name="Lebanon" id="LB" style="display: none;" selectable>
                                <option value="Asia/Beirut">Beirut</option>
                            </select>

                            <select name="Lesotho" id="LS" style="display: none;" selectable>
                                <option value="Africa/Maseru">Maseru</option>
                            </select>

                            <select name="Liberia" id="LR" style="display: none;" selectable>
                                <option value="Africa/Monrovia">Monrovia</option>
                            </select>

                            <select name="Libya" id="LY" style="display: none;" selectable>
                                <option value="Africa/Tripoli">Tripoli</option>
                            </select>

                            <select name="Liechtenstein" id="LI" style="display: none;" selectable>
                                <option value="Europe/Vaduz">Vaduz</option>
                            </select>

                            <select name="Lithuania" id="LT" style="display: none;" selectable>
                                <option value="Europe/Vilnius">Vilnius</option>
                            </select>

                            <select name="Luxembourg" id="LU" style="display: none;" selectable>
                                <option value="Europe/Luxembourg">Luxembourg</option>
                            </select>

                            <select name="Macao" id="MO" style="display: none;" selectable>
                                <option value="Asia/Macau">Macau</option>
                            </select>

                            <select name="Macedonia, the Former Yugoslav Republic of" id="MK" style="display: none;" selectable>
                                <option value="Europe/Skopje">Skopje</option>
                            </select>

                            <select name="Madagascar" id="MG" style="display: none;" selectable>
                                <option value="Indian/Antananarivo">Antananarivo</option>
                            </select>

                            <select name="Malawi" id="MW" style="display: none;" selectable>
                                <option value="Africa/Blantyre">Blantyre</option>
                            </select>

                            <select name="Malaysia" id="MY" style="display: none;" selectable>
                                <option value="Asia/Kuala_Lumpur">Kuala Lumpur</option>
                                <option value="Asia/Kuching">Kuching</option>
                            </select>

                            <select name="Maldives" id="MV" style="display: none;" selectable>
                                <option value="Indian/Maldives">Maldives</option>
                            </select>

                            <select name="Mali" id="ML" style="display: none;" selectable>
                                <option value="Africa/Bamako">Bamako</option>
                            </select>

                            <select name="Malta" id="MT" style="display: none;" selectable>
                                <option value="Europe/Malta">Malta</option>
                            </select>

                            <select name="Marshall Islands" id="MH" style="display: none;" selectable>
                                <option value="Pacific/Kwajalein">Kwajalein</option>
                                <option value="Pacific/Majuro">Majuro</option>
                            </select>

                            <select name="Martinique" id="MQ" style="display: none;" selectable>
                                <option value="America/Martinique">Martinique</option>
                            </select>

                            <select name="Mauritania" id="MR" style="display: none;" selectable>
                                <option value="Africa/Nouakchott">Nouakchott</option>
                            </select>

                            <select name="Mauritius" id="MU" style="display: none;" selectable>
                                <option value="Indian/Mauritius">Mauritius</option>
                            </select>

                            <select name="Mayotte" id="YT" style="display: none;" selectable>
                                <option value="Indian/Mayotte">Mayotte</option>
                            </select>

                            <select name="Mexico" id="MX" style="display: none;" selectable>
                                <option value="America/Bahia_Banderas">Bahia Banderas</option>
                                <option value="America/Cancun">Cancun</option>
                                <option value="America/Chihuahua">Chihuahua</option>
                                <option value="America/Hermosillo">Hermosillo</option>
                                <option value="America/Matamoros">Matamoros</option>
                                <option value="America/Mazatlan">Mazatlan</option>
                                <option value="America/Merida">Merida</option>
                                <option value="America/Mexico_City">Mexico City</option>
                                <option value="America/Monterrey">Monterrey</option>
                                <option value="America/Ojinaga">Ojinaga</option>
                                <option value="America/Tijuana">Tijuana</option>
                            </select>

                            <select name="Micronesia, Federated States of" id="FM" style="display: none;" selectable>
                                <option value="Pacific/Chuuk">Chuuk</option>
                                <option value="Pacific/Kosrae">Kosrae</option>
                                <option value="Pacific/Pohnpei">Pohnpei</option>
                            </select>

                            <select name="Moldova, Republic of" id="MD" style="display: none;" selectable>
                                <option value="Europe/Chisinau">Chisinau</option>
                            </select>

                            <select name="Monaco" id="MC" style="display: none;" selectable>
                                <option value="Europe/Monaco">Monaco</option>
                            </select>

                            <select name="Mongolia" id="MN" style="display: none;" selectable>
                                <option value="Asia/Choibalsan">Choibalsan</option>
                                <option value="Asia/Hovd">Hovd</option>
                                <option value="Asia/Ulaanbaatar">Ulaanbaatar</option>
                            </select>

                            <select name="Montenegro" id="ME" style="display: none;" selectable>
                                <option value="Europe/Podgorica">Podgorica</option>
                            </select>

                            <select name="Montserrat" id="MS" style="display: none;" selectable>
                                <option value="America/Montserrat">Montserrat</option>
                            </select>

                            <select name="Morocco" id="MA" style="display: none;" selectable>
                                <option value="Africa/Casablanca">Casablanca</option>
                            </select>

                            <select name="Mozambique" id="MZ" style="display: none;" selectable>
                                <option value="Africa/Maputo">Maputo</option>
                            </select>

                            <select name="Myanmar" id="MM" style="display: none;" selectable>
                                <option value="Asia/Yangon">Yangon</option>
                            </select>

                            <select name="Namibia" id="NA" style="display: none;" selectable>
                                <option value="Africa/Windhoek">Windhoek</option>
                            </select>

                            <select name="Nauru" id="NR" style="display: none;" selectable>
                                <option value="Pacific/Nauru">Nauru</option>
                            </select>

                            <select name="Nepal" id="NP" style="display: none;" selectable>
                                <option value="Asia/Kathmandu">Kathmandu</option>
                            </select>

                            <select name="Netherlands" id="NL" style="display: none;" selectable>
                                <option value="Europe/Amsterdam">Amsterdam</option>
                            </select>

                            <select name="New Caledonia" id="NC" style="display: none;" selectable>
                                <option value="Pacific/Noumea">Noumea</option>
                            </select>

                            <select name="New Zeland" id="NZ" style="display: none;" selectable>
                                <option value="Pacific/Auckland">Auckland</option>
                                <option value="Pacific/Chatham">Chatham</option>
                            </select>

                            <select name="Nicaragua" id="NI" style="display: none;" selectable>
                                <option value="America/Managua">Managua</option>
                            </select>

                            <select name="Niger" id="NE" style="display: none;" selectable>
                                <option value="Africa/Niamey">Niamey</option>
                            </select>

                            <select name="Nigeria" id="NG" style="display: none;" selectable>
                                <option value="Africa/Lagos">Lagos</option>
                            </select>

                            <select name="Niue" id="NU" style="display: none;" selectable>
                                <option value="Pacific/Niue">Niue</option>
                            </select>

                            <select name="Norfolk Island" id="NF" style="display: none;" selectable>
                                <option value="Pacific/Norfolk">Norfolk</option>
                            </select>

                            <select name="Nothern Mariana Islands" id="MP" style="display: none;" selectable>
                                <option value="Pacific/Saipan">Saipan</option>
                            </select>

                            <select name="Norway" id="NO" style="display: none;" selectable>
                                <option value="Europe/Oslo">Oslo</option>
                            </select>

                            <select name="Oman" id="OM" style="display: none;" selectable>
                                <option value="Asia/Muscat">Muscat</option>
                            </select>

                            <select name="Pakistan" id="PK" style="display: none;" selectable>
                                <option value="Asia/Karachi">Karachi</option>
                            </select>

                            <select name="Palau" id="PW" style="display: none;" selectable>
                                <option value="Pacific/Palau">Palau</option>
                            </select>

                            <select name="Palestine, State of" id="PS" style="display: none;" selectable>
                                <option value="Asia/Gaza">Gaza</option>
                                <option value="Asia/Hebron">Hebron</option>
                            </select>

                            <select name="Panama" id="PA" style="display: none;" selectable>
                                <option value="America/Panama">Panama</option>
                            </select>

                            <select name="Papua New Guinea" id="PG" style="display: none;" selectable>
                                <option value="Pacific/Bougainville">Bougainville</option>
                                <option value="Pacific/Port_Moresby">Port Moresby</option>
                            </select>

                            <select name="Paraguay" id="PY" style="display: none;" selectable>
                                <option value="America/Asuncion">Asuncion</option>
                            </select>

                            <select name="Peru" id="PE" style="display: none;" selectable>
                                <option value="America/Lima">Lima</option>
                            </select>

                            <select name="Philippines" id="PH" style="display: none;" selectable>
                                <option value="Asia/Manila">Manila</option>
                            </select>

                            <select name="Pitcairn" id="PN" style="display: none;" selectable>
                                <option value="Pacific/Pitcairn">Pitcairn</option>
                            </select>

                            <select name="Poland" id="PL" style="display: none;" selectable>
                                <option value="Europe/Warsaw">Warsaw</option>
                            </select>

                            <select name="Portugal" id="PT" style="display: none;" selectable>
                                <option value="Atlantic/Azores">Azores</option>
                                <option value="Atlantic/Madeira">Madeira</option>
                                <option value="Europe/Lisbon">Lisbon</option>
                            </select>

                            <select name="Porto Rico" id="PR" style="display: none;" selectable>
                                <option value="America/Puerto_Rico">Porto Rico</option>
                            </select>

                            <select name="Qatar" id="QA" style="display: none;" selectable>
                                <option value="Asia/Qatar">Qatar</option>
                            </select>

                            <select name="Romania" id="RO" style="display: none;" selectable>
                                <option value="Europe/Bucharest">Bucharest</option>
                            </select>

                            <select name="Russian Federation" id="RU" style="display: none;" selectable>
                                <option value="Asia/Anadyr">Anadyr</option>
                                <option value="Asia/Barnaul">Barnaul</option>
                                <option value="Asia/Chita">Chita</option>
                                <option value="Asia/Irkutsk">Irkutsk</option>
                                <option value="Asia/Kamchatka">Kamchatka</option>
                                <option value="Asia/Khandyga">Khandyga</option>
                                <option value="Asia/Krasnoyarsk">Krasnoyarsk</option>
                                <option value="Asia/Magadan">Magadan</option>
                                <option value="Asia/Novokuznetsk">Novokuznetsk</option>
                                <option value="Asia/Novosibirsk">Novosibirsk</option>
                                <option value="Asia/Omsk">Omsk</option>
                                <option value="Asia/Sakhalin">Sakhalin</option>
                                <option value="Asia/Srednekolymsk">Srednekolymsk</option>
                                <option value="Asia/Tomsk">Tomsk</option>
                                <option value="Asia/Ust-Nera">Ust Nera</option>
                                <option value="Asia/Vladivostok">Vladivostok</option>
                                <option value="Asia/Yakutsk">Yakutsk</option>
                                <option value="Asia/Yekaterinburg">Yekaterinburg</option>
                                <option value="Europe/Astrakhan">Astrakhan</option>
                                <option value="Europe/Kaliningrad">Kaliningrad</option>
                                <option value="Europe/Kirov">Kirov</option>
                                <option value="Europe/Moscow">Moscow</option>
                                <option value="Europe/Samara">Samara</option>
                                <option value="Europe/Saratov">Saratov</option>
                                <option value="Europe/Ulyanovsk">Ulyanovsk</option>
                                <option value="Europe/Volgograd">Volgograd</option>
                            </select>

                            <select name="Rwanda" id="RW" style="display: none;" selectable>
                                <option value="Africa/Kigali">Kigali</option>
                            </select>

                            <select name="Reunion" id="RE" style="display: none;" selectable>
                                <option value="Indian/Reunion">Reunion</option>
                            </select>

                            <select name="Saint Barthelemy" id="BL" style="display: none;" selectable>
                                <option value="America/St_Barthelemy">Saint Barthélemy</option>
                            </select>

                            <select name="Saint Helena, Ascension and Tristan da Cunha" id="SH" style="display: none;" selectable>
                                <option value="Atlantic/St_Helena">Saint Helena</option>
                            </select>

                            <select name="Saint Kitts and Nevis" id="KN" style="display: none;" selectable>
                                <option value="America/St_Kitts">Saint Kitts</option>
                            </select>

                            <select name="Saint Lucia" id="LC" style="display: none;" selectable>
                                <option value="America/St_Lucia">Saint Lucia</option>
                            </select>

                            <select name="Saint Martin (French part)" id="MF" style="display: none;" selectable>
                                <option value="America/Marigot">Marigot</option>
                            </select>

                            <select name="Saint Pierre and Miquelon" id="PM" style="display: none;" selectable>
                                <option value="America/Miquelon">Miquelon</option>
                            </select>

                            <select name="Saint Vincent and the Grenadines" id="VC" style="display: none;" selectable>
                                <option value="America/St_Vincent">Saint Vincent</option>
                            </select>

                            <select name="Samoa" id="WS" style="display: none;" selectable>
                                <option value="Pacific/Apia">Apia</option>
                            </select>

                            <select name="San Marino" id="SM" style="display: none;" selectable>
                                <option value="Europe/San_Marino">San Marino</option>
                            </select>

                            <select name="Sao Tome and Principe" id="ST" style="display: none;" selectable>
                                <option value="Africa/Sao_Tome">São Tome</option>
                            </select>

                            <select name="Saudi Arabia" id="SA" style="display: none;" selectable>
                                <option value="Asia/Riyadh">Riyadh</option>
                            </select>

                            <select name="Senegal" id="SN" style="display: none;" selectable>
                                <option value="Africa/Dakar">Dakar</option>
                            </select>

                            <select name="Serbia" id="RS" style="display: none;" selectable>
                                <option value="Europe/Belgrade">Belgrade</option>
                            </select>

                            <select name="Seychelles" id="SC" style="display: none;" selectable>
                                <option value="Indian/Mahe">Mahe</option>
                            </select>

                            <select name="Sierra Leone" id="SL" style="display: none;" selectable>
                                <option value="Africa/Freetown">Freetown</option>
                            </select>

                            <select name="Singapore" id="SG" style="display: none;" selectable>
                                <option value="Asia/Singapore">Singapore</option>
                            </select>

                            <select name="Sint Maarten (Dutch part)" id="SX" style="display: none;" selectable>
                                <option value="America/Lower_Princes">Lower Princes</option>
                            </select>

                            <select name="Slovakia" id="SK" style="display: none;" selectable>
                                <option value="Europe/Bratislava">Bratislava</option>
                            </select>

                            <select name="Slovenia" id="SI" style="display: none;" selectable>
                                <option value="Europe/Ljubljana">Ljubljana</option>
                            </select>

                            <select name="Solomon Islands" id="SB" style="display: none;" selectable>
                                <option value="Pacific/Guadalcanal">Guadalcanal</option>
                            </select>

                            <select name="Somalia" id="SO" style="display: none;" selectable>
                                <option value="Africa/Mogadishu">Mogadishu</option>
                            </select>

                            <select name="South Africa" id="ZA" style="display: none;" selectable>
                                <option value="Africa/Johannesburg">Johannesburg</option>
                            </select>

                            <select name="South Georgia and the South Sandwich" id="GS" style="display: none;" selectable>
                                <option value="Atlantic/South_Georgia">South Georgia</option>
                            </select>

                            <select name="South Sudan" id="SS" style="display: none;" selectable>
                                <option value="Africa/Juba">Juba</option>
                            </select>

                            <select name="Spain" id="ES" style="display: none;" selectable>
                                <option value="Africa/Ceuta">Ceuta</option>
                                <option value="Atlantic/Canary">Canary</option>
                                <option value="Europe/Madrid">Madrid</option>
                            </select>

                            <select name="Sri Lanka" id="LK" style="display: none;" selectable>
                                <option value="Asia/Colombo">Colombo</option>
                            </select>

                            <select name="Sudan" id="SD" style="display: none;" selectable>
                                <option value="Africa/Khartoum">Khartoum</option>
                            </select>

                            <select name="Suriname" id="SR" style="display: none;" selectable>
                                <option value="America/Paramaribo">Paramaribo</option>
                            </select>

                            <select name="Svalbard and Jan Mayen" id="SJ" style="display: none;" selectable>
                                <option value="Arctic/Longyearbyen">Longyearbyen</option>
                            </select>

                            <select name="Swaziland" id="SZ" style="display: none;" selectable>
                                <option value="Africa/Mbabane">Mbabane</option>
                            </select>

                            <select name="Sweden" id="SE" style="display: none;" selectable>
                                <option value="Europe/Stockholm">Stockholm</option>
                            </select>

                            <select name="Switzerland" id="CH" style="display: none;" selectable>
                                <option value="Europe/Zurich">Zurich</option>
                            </select>

                            <select name="Syrian Arab Republic" id="SY" style="display: none;" selectable>
                                <option value="Asia/Damascus">Damascus</option>
                            </select>

                            <select name="Taiwan, Province of China" id="TW" style="display: none;" selectable>
                                <option value="Asia/Taipei">Taipei</option>
                            </select>

                            <select name="Tajikistan" id="TJ" style="display: none;" selectable>
                                <option value="Asia/Dushanbe">Dushanbe</option>
                            </select>

                            <select name="Thailand" id="TH" style="display: none;" selectable>
                                <option value="Asia/Bangkok">Bangkok</option>
                            </select>

                            <select name="Timor-Leste" id="TL" style="display: none;" selectable>
                                <option value="Asia/Dili">Dili</option>
                            </select>

                            <select name="Togo" id="TG" style="display: none;" selectable>
                                <option value="Africa/Lome">Lome</option>
                            </select>

                            <select name="Tokelau" id="TK" style="display: none;" selectable>
                                <option value="Pacific/Fakaofo">Fakaofo</option>
                            </select>

                            <select name="Tonga" id="TO" style="display: none;" selectable>
                                <option value="Pacific/Tongatapu">Tongatapu</option>
                            </select>

                            <select name="Trinidad and Tobago" id="TT" style="display: none;" selectable>
                                <option value="America/Port_of_Spain">Port of Spain</option>
                            </select>

                            <select name="Tunisia" id="TN" style="display: none;" selectable>
                                <option value="Africa/Tunis">Tunis</option>
                            </select>

                            <select name="Turkey" id="TR" style="display: none;" selectable>
                                <option value="Europe/Istanbul">Istanbul</option>
                            </select>

                            <select name="Turkmenistan" id="TM" style="display: none;" selectable>
                                <option value="Asia/Ashgabat">Ashgabat</option>
                            </select>

                            <select name="Turks and Caicos Islands" id="TC" style="display: none;" selectable>
                                <option value="America/Grand_Turk">Grand Turk</option>
                            </select>

                            <select name="Tuvalu" id="TV" style="display: none;" selectable>
                                <option value="Pacific/Funafuti">Funafuti</option>
                            </select>

                            <select name="Uganda" id="UG" style="display: none;" selectable>
                                <option value="Africa/Kampala">Kampala</option>
                            </select>

                            <select name="Ukraine" id="UA" style="display: none;" selectable>
                                <option value="Europe/Kiev">Kiev</option>
                                <option value="Europe/Simferopol">Simferopol</option>
                                <option value="Europe/Uzhgorod">Uzhgorod</option>
                                <option value="Europe/Zaporozhye">Zaporozhye</option>
                            </select>

                            <select name="United Arab Emirates" id="AE" style="display: none;" selectable>
                                <option value="Asia/Dubai">Dubai</option>
                            </select>

                            <select name="United Kingdom" id="GB" style="display: none;" selectable>
                                <option value="Europe/London">London</option>
                            </select>

                            <select name="United States" id="US" style="display: none;" selectable>
                                <option value="America/Adak">Adak</option>
                                <option value="America/Anchorage">Anchorage</option>
                                <option value="America/Boise">Boise</option>
                                <option value="America/Chicago">Chicago</option>
                                <option value="America/Denver">Denver</option>
                                <option value="America/Detroit">Detroit</option>
                                <option value="America/Indiana/Indianapolis">Indiana - Indianapolis</option>
                                <option value="America/Indiana/Knox">Indiana - Knox</option>
                                <option value="America/Indiana/Marengo">Indiana - Marengo</option>
                                <option value="America/Indiana/Petersburg">Indiana - Petersburg</option>
                                <option value="America/Indiana/Tell_City">Indiana - Tell City</option>
                                <option value="America/Indiana/Vevay">Indiana - Vevay</option>
                                <option value="America/Indiana/Vincennes">Indiana - Vincennes</option>
                                <option value="America/Indiana/Winamac">Indiana - Winamac</option>
                                <option value="America/Juneau">Juneau</option>
                                <option value="America/Kentucky/Louisville">Kentucky - Louisville</option>
                                <option value="America/Kentucky/Monticello">Kentucky - Monticello</option>
                                <option value="America/Los_Angeles">Los Angeles</option>
                                <option value="America/Menominee">Menominee</option>
                                <option value="America/Metlakatla">Metlakatla</option>
                                <option value="America/New_York">New York</option>
                                <option value="America/Nome">Nome</option>
                                <option value="America/North_Dakota/Beulah">North Dakota - Beulah</option>
                                <option value="America/North_Dakota/Center">North Dakota - Center</option>
                                <option value="America/North_Dakota/New_Salem">North Dakota - New Salem</option>
                                <option value="America/Phoenix">Phoenix</option>
                                <option value="America/Sitka">Sitka</option>
                                <option value="America/Yakutat">Yakutat</option>
                                <option value="Pacific/Honolulu">Honolulu</option>
                            </select>

                            <select name="United States Minor Outlying Islands" id="UM" style="display: none;" selectable>
                                <option value="Pacific/Midway">Midway</option>
                                <option value="Pacific/Wake">Wake</option>
                            </select>

                            <select name="Uruguay" id="UY" style="display: none;" selectable>
                                <option value="America/Montevideo">Montevideo</option>
                            </select>

                            <select name="Uzbekistan" id="UZ" style="display: none;" selectable>
                                <option value="Asia/Samarkand">Samarkand</option>
                                <option value="Asia/Tashkent">Tashkent</option>
                            </select>

                            <select name="Vanuatu" id="VU" style="display: none;" selectable>
                                <option value="Pacific/Efate">Efate</option>
                            </select>

                            <select name="Venezuela, Bolivarian Republic of" id="VE" style="display: none;" selectable>
                                <option value="America/Caracas">Caracas</option>
                            </select>

                            <select name="Viet Nam" id="VN" style="display: none;" selectable>
                                <option value="Asia/Ho_Chi_Minh">Ho Chi Minh</option>
                            </select>

                            <select name="Virgin Islands, British" id="VG" style="display: none;" selectable>
                                <option value="America/Tortola">Tortola</option>
                            </select>

                            <select name="Virgin Islands, U.S." id="VI" style="display: none;" selectable>
                                <option value="America/St_Thomas">Saint Thomas</option>
                            </select>

                            <select name="Wallis and Futuna" id="WF" style="display: none;" selectable>
                                <option value="Pacific/Wallis">Wallis</option>
                            </select>

                            <select name="Western Sahara" id="EH" style="display: none;" selectable>
                                <option value="Africa/El_Aaiun">El Aaiun</option>
                            </select>

                            <select name="Yemen" id="YE" style="display: none;" selectable>
                                <option value="Asia/Aden">Aden</option>
                            </select>

                            <select name="Zambia" id="ZM" style="display: none;" selectable>
                                <option value="Africa/Lusaka">Lusaka</option>
                            </select>

                            <select name="Zimbabwe" id="ZW" style="display: none;" selectable>
                                <option value="Africa/Harare">Harare</option>
                            </select>

                            <select name="Aland Islands" id="AX" style="display: none;" selectable>
                                <option value="Europe/Mariehamn">Mariehamn</option>
                            </select>
                        </span>
                    </div>
                    </fieldset>
                <fieldset>
                    <div class="move-time">
                        <span>  
                            <input type="checkbox" id="move-time"> Move Time
                        </span>
                        <br>
                            <span id="show-movetime" style="display: none;">
                                <span>
                                    <input type="number" value="0" id="hour-move" placeholder="h">
                                    <label > hour</label><br>
                                    <input type="number" value="0" id="min-move" placeholder="m">
                                    <label >min</label><br>
                                    <input type="number" value="0" id="sec-move" placeholder="s">
                                    <label>sec</label><br>
                                </span>
                                <br>
                                    <span>
                                        <input type="radio" name="move-time" value="delay"> Delay
                                        <input type="radio" name="move-time" value="antecipate"> Antecipate
                                    </span>
                            </span>    
                    </div>
                </fieldset>
                <fieldset>
                    <span>
                        <br>
                        <input type="checkbox" name="auto-set-off" id="auto-set-off"> Turn Off Auto Set Clock
                        <br>
                    </span>
                    <br>
                        <span id="show-turned-off-auto-set" style="display: none;">
                            <label>Manual Set Clock</label><br>
                            <input type="time" step="1" id="time"><br>
                            <label>hour : min : sec</label>
                            <br>
                            <br>
                                <span id="noon">
                                    <span>
                                        <input type="radio" name="noon" value="0"> AM
                                        <input type="radio" name="noon" value="1"> PM
                                    </span>
                                    <br>
                                    <br>
                                </span>
                                <label>Manual Set Date</label><br>
                                <input type="date" id="date"> 
                        </span>
                </fieldset>
                    <div>
                        <label for="ntpServer">NTP Server</label>
                        <br>
                            <select id="ntp" name="ntp">
                                <option value="africa.pool.ntp.org">Africa</option>
                                <option value="asia.pool.ntp.org">Asia</option>
                                <option value="europe.pool.ntp.org">Europe</option>
                                <option value="north-america.pool.ntp.org">North America</option>
                                <option value="oceania.pool.ntp.org">Oceania</option>
                                <option value="south-america.pool.ntp.org">South America</option>
                            </select>
                    </div>
                <fieldset>
                    <label>Date Type</label>
                        <br>
                    <div class="date-type">
                        <input type="radio" name="date-type" value="0"> DD/MM/YYYY
                        <br>
                        <input type="radio" name="date-type" value="1"> MM/DD/YYYY
                        <br>
                        <input type="radio" name="date-type" value="2"> YYYY-MM-DD
                    </div>
                </fieldset>
            </fieldset>
        </form>
    </body>
    <blank-area1>

    </blank-area1>
    <clock class="tag">
        <div class="dateTimeInfo" >
            <div>
                <label id="page-clock" for="time" class="time">Page Clock</label>
            </div>
            <div class="espaco"></div>
            <div>
                <label id="calendar" for="date" class="date">Waiting For Server</label>
            </div>
        </div>
    </clock>
    <footer>
        <div class="buttons">
            <button class="primary" id="savebtn" type="button" onclick="saveFunction()">SAVE</button>
            <div class="espaco"></div>
            <button class="primary" id="resetbtn" type="button" onclick="resetFunction()">RESET</button>
        </div>
    </footer>
    <script>

        const country = document.getElementById('Country')                      // Váriável de controle do select país(Country)
        const moveTime = document.querySelector("#move-time")                   // Váriável de controle do checkbox move-time (Deslocamento de tempo)
        const turnOffAutoSet = document.querySelector("#auto-set-off")          // Váriável de controle do checkbox auto-set-off (Ajuste manual do relógio)
        let timeFormat = document.getElementsByName("time-format")              // Variável de controle do radio-button (time-format)
        let moveTimeDelayAntecipate = document.getElementsByName("move-time")   // Variável de controle do radio-button (move-time)
        let noon = document.getElementsByName("noon")                           // Váriavel de controle do radio-button (noon)
        const ntp = document.getElementById('ntp')                              // Váriável de controle do select de servidores ntp (ntp)
        let dateType = document.getElementsByName("date-type")                  // Variável de controle do radio-button (date-type)
        const clock = document.getElementById('page-clock')                     // Váriável de controle do relógio da página
        const calendar = document.getElementById('calendar')                    // Váriavel de controle do calendário da página

        // Váriaveis de controle do reset
        let resetControl = 0
        let tempResetControl

        // Carrega configurações salvas no microcontrolador
        function ajax(config) {
            const xhr = new XMLHttpRequest()
            xhr.open(config.method, config.url, true)

            xhr.onload = e => {
                if (xhr.status === 200) {
                    config.success(xhr.response)
                } else if (xhr.status >= 400) {
                    config.notSuccess({
                        code: xhr.status,
                        text: xhr.statusText
                    })
                }
            }

            xhr.send()
        }

        
        ajax({
            url: "/load",
            method: "get",
            success(answer) {
                let config = answer

                if(config === 'reset') {
                    resetFunction()
                } else {
                    config = config.split(",")

                    // Disposição do array

                    // ssid, config[0]
                    // pass, config[1]
                    // displayTimeFormat, config[2]
                    // timeCountry, config[3]                
                    // timeCountryPartialLink, config[4]
                    // moveTime, config[5]
                    // moveHour, config[6]
                    // moveMin, config[7]
                    // moveSec, config[8]
                    // moveSignal, config[9]
                    // manualAjust, config[10]
                    // noonMode, config[11]
                    // ntpServer, config[12]
                    // dateType, config[13]
                    // resetControl, config[14]

                    // Seta configurações dos dados da rede
                    document.getElementById("ssid").value = config[0]
                    document.getElementById("password").value = config[1]


                    // Seta configuração do dado do modo de exibição do horário
                    if (config[2] == 12) {
                        timeFormat[0].checked = true
                    } else if (config[2] == 24) {
                        timeFormat[1].checked = true
                    }


                    // // Seta Key do timezonedb.com
                    // document.getElementById("key").value = config[]


                    // Seta configuração do País e fuso
                    document.getElementById('Country').value = config[3]
                    document.querySelectorAll('[selectable]').forEach(country => {

                    country.style.display = 'none'
                    if (country.id == document.getElementById("Country").value) {
                        country.value = config[4]
                        country.style.display = 'block'
                    }       
                    })



                    // Seta configuração do deslocamento do tempo
                    if (config[5] === 'false') {
                        moveTime.checked = false
                    } else {
                        moveTime.checked = true
                    }
                    
                    document.getElementById("hour-move").value = config[6]
                    document.getElementById("min-move").value = config[7]
                    document.getElementById("sec-move").value = config[8]

                    if (config[9] == 0) {
                        moveTimeDelayAntecipate[1].checked = true
                    } else if (config[9] == 1) {
                        moveTimeDelayAntecipate[0].checked = true
                    }
                    
                    // if (config[] === 'false') {
                    //     turnoffAutoTZ.checked = false
                    // } else {
                    //     turnoffAutoTZ.checked = true
                    // }
                    

                    // Seta o ajuste manual
                    if (config[10] === 'false') {
                        turnOffAutoSet.checked = false
                    } else {
                        turnOffAutoSet.checked = true
                    }
                    
                    document.getElementById("time").value = ""

                    if (config[11] == 0) {
                        noon[0].checked = true
                    } else if (config[11] == 1) {
                        noon[1].checked = true
                    }

                    document.getElementById("date").value = ""


                    // Seta NTP Server
                    document.getElementById('ntp').value = config[12]


                    // Seta o formato da data
                    if (config[13] == '0') {
                        dateType[0].checked = true
                    } else if (config[13] == '1') {
                        dateType[1].checked = true
                    } else {
                        dateType[2].checked = true
                    }
                    

                    // 
                    if (config[14] == 0) {
                        tempResetControl = 0
                    } else if (config[14] == 1) {
                        tempResetControl = 1
                    }

                    // Atualiza a página para mostrar configurações setadas
                    loadPage()

                }
            },
            notSuccess(e) {
                const msg = document.createTextNode(`${e.code}: ${e.text}`)
                console.log(msg)
            }
        })

        function loadPage() {
            

            // Ao carregar a página seleciona a timezone em relação ao país selecionado
            document.querySelectorAll('[selectable]').forEach(country => {

            country.style.display = 'none'
            if (country.id == document.getElementById("Country").value) {
                country.style.display = 'block'
            }       
            })


            // Ao carregar a página mostra ou esconde os campos de deslocamento de tempo dependendo se está opção está selecionada
            if (moveTime.checked) {
            document.getElementById('show-movetime').style.display = 'block'
            } else {
            document.getElementById('show-movetime').style.display = 'none'
            }


            // Ao carregar a página mostra ou esconde os campos relacionados ao ajuste manual do relógio
            if (turnOffAutoSet.checked) {
                document.getElementById('show-turned-off-auto-set').style.display = 'block'
            } else {
                document.getElementById('show-turned-off-auto-set').style.display = 'none'
            }


            // Ao carregar se nenhum radio-button de time-format estiver selexionado ocorre a seleção do modo 24 horas
            if (!timeFormat[0].checked && !timeFormat[1].checked) {
            timeFormat[1].checked = true
            }


            // Ao carregar a páginá mostra ou esconde os radio-button AM e PM
            if (timeFormat[1].checked) {
            document.getElementById('noon').style.display = 'none'
            } else {
            document.getElementById('noon').style.display = 'block'
            }



            // Ao carregar se nenhum radio-button de move-time estiver selexionado ocorre a seleção do delay
            if (!moveTimeDelayAntecipate[0].checked && !moveTimeDelayAntecipate[1].checked) {
            moveTimeDelayAntecipate[0].checked = true
            }


            // Ao carregar se nenhum radio-button de noon estiver selexionado ocorre a seleção do AM
            if (!noon[0].checked && !noon[1].checked) {
            noon[0].checked = true
            }


            // Ao carregar se nenhum radio-button de date-type estiver selecionado ocorre a seleção do modo DD/MM/AAAA
            if (!dateType[0].checked && !dateType[1].checked && !dateType[2].checked) {
               dateType[0].checked = true
            }

        }

        // Aplica as configurações iniciais da página
        loadPage()


        // Ao escolher um país define qual timezone será exibida
        country.addEventListener('change', function() {
            document.querySelectorAll('[selectable]').forEach(country => {

            country.style.display = 'none'
            if (country.id == document.getElementById("Country").value) {
                country.style.display = 'block'
            }       
            })
        })


        // Ao marcar/desmarcar a opção de deslocamento de tempo exibe/esconde os campos relacionados
        moveTime.addEventListener("change", (el) => {
            if (moveTime.checked) {
                document.getElementById('show-movetime').style.display = 'block'
            } else {
                document.getElementById('show-movetime').style.display = 'none'
            }
        })


        // Ao marcar/desmarcar a opção de desligar ajuste automático do relógio exibe/esconde os campos relacionados
        turnOffAutoSet.addEventListener("change", (el) => {
            if (turnOffAutoSet.checked) {
                document.getElementById('show-turned-off-auto-set').style.display = 'block'
            } else {
                document.getElementById('show-turned-off-auto-set').style.display = 'none'
            }
        })


        // Ao clicar em um dos radio-buttons de time-format exibe/esconde os radio-button de AM e PM
        timeFormat[0].addEventListener("change", (el) => {
            if (timeFormat[1].checked) {
                document.getElementById('noon').style.display = 'none'
            } else {
                document.getElementById('noon').style.display = 'block'
        }
        })

        timeFormat[1].addEventListener("change", (el) => {
            if (timeFormat[1].checked) {
                document.getElementById('noon').style.display = 'none'
            } else {
                document.getElementById('noon').style.display = 'block'
        }
        })

        
        // Função do botão Save
        function saveFunction() {

            console.log("save button was clicked!")

            // Dados da rede
            let ssid = document.getElementById("ssid").value
            let password = document.getElementById("password").value
            

            // Dado do formato de exibição do horário (12 ou 24 horas)
            let displayTimeFormat
            if (timeFormat[0].checked) {
                displayTimeFormat = timeFormat[0].value
            } else {
                displayTimeFormat = timeFormat[1].value
            }


            // Dados do país e do fuso-horário
            let timeCountry = document.getElementById("Country").value
            let timeCountryName
            let timeCountryLink = 'http://worldtimeapi.org/api/timezone/'
            let timeCountryPartialLink
            document.querySelectorAll('[selectable]').forEach(country => {
                if (country.id == document.getElementById("Country").value) {
                    timeCountryName = country.name
                    timeCountryLink += country.value
                    timeCountryPartialLink = country.value
                }       
            })


            // Dados do deslocamento do horário
            let moveTimeSelected = moveTime.checked
            let moveHour = document.getElementById("hour-move").value
            let moveMin = document.getElementById("min-move").value
            let moveSec = document.getElementById("sec-move").value
            if (moveHour === "") {
                moveHour = 0
            }
            if (moveMin === "") {
                moveMin = 0
            }
            if (moveSec === "") {
                moveSec = 0
            }
            let moveSignal
            if (moveTimeDelayAntecipate[0].checked) {
                moveSignal = 1
            } else {
                moveSignal = 0
            }


            // Transforma strings do moveTime em inteiros
            moveHour = parseInt(moveHour)
            moveMin = parseInt(moveMin)
            moveSec = parseInt(moveSec)


            // Dados de ajuste manual do relógio
            let manualAjust = turnOffAutoSet.checked
            let manualTime = document.getElementById("time").value
            let manualDate = document.getElementById("date").value
            let now = new Date          // Está data/hora será usada caso seja ativado o ajuste manual e o usuário não forneça data e/ou hora
            let manualHour
            let manualMin
            let manualSec
            let manualYear
            let manualMonth
            let manualDay
            let noonMode

            // Se não fornecido pega horário do compuador
            if (manualTime === "") {
                manualHour = now.getHours()
                manualMin = now.getMinutes()
                manualSec = now.getSeconds()
            } else {
                manualTime = manualTime.split(':')
                manualHour = manualTime[0]
                manualMin = manualTime[1]
                manualSec = manualTime[2]
            }

            // Se não for fornecido pega data do computador
            if (manualDate === "") {
                manualYear = now.getFullYear()
                manualMonth = now.getMonth() + 1
                manualDay = now.getDate()
            } else {
                manualDate = manualDate.split('-')
                manualYear = manualDate[0]
                manualMonth = manualDate[1]
                manualDay = manualDate[2]
            }
            

            // Transforma as Strings de data em inteiros
            manualHour = parseInt(manualHour)
            manualMin = parseInt(manualMin)
            manualSec = parseInt(manualSec)

            manualDay = parseInt(manualDay)
            manualMonth = parseInt(manualMonth)
            manualYear = parseInt(manualYear)

            
            // Corrige pra AM em caso de horas "Zero" ou "Doze" e em caso de PM correto transforma em notação 24 horas
            if (displayTimeFormat == 12) {
                if (manualHour == 0 || manualHour == 12) {
                    noonMode = 0
                    noon[0].checked = true
                }

                if (noon[1].checked) {
                    noonMode = 1
                    manualHour += 12
                }
            }


            // Caso não setado, seta noonMode
            if (noonMode === undefined) {
                noonMode = 0
            }


            // Dado do servidor NTP
            let ntpServer = document.getElementById("ntp").value


            // Dado do formato de exibição da data (DD/MM/AAAA, MM/DD/AAAA, AAAA-MM-DD)
            let dateTypeFormat
            if (dateType[0].checked) {
                dateTypeFormat = dateType[0].value
            } else if (dateType[1].checked) {
                dateTypeFormat = dateType[1].value
            } else {
                dateTypeFormat = dateType[2].value
            }            
            
            // Objeto com as informações que seram passadas ao microcontrolador
            let data = {ssid,
                        password,
                        displayTimeFormat,
                        timeCountry,
                        timeCountryName,
                        timeCountryLink,
                        timeCountryPartialLink,
                        moveTime: moveTimeSelected,
                        moveHour,
                        moveMin,
                        moveSec,
                        moveSignal,
                        manualAjust,
                        noonMode,
                        manualHour,
                        manualMin,
                        manualSec,
                        manualDay,
                        manualMonth,
                        manualYear,
                        ntpServer,
                        dateType: dateTypeFormat,
                        resetControl}

            console.log(data)
        
            // Manda os dados para o microcontrolador
            let xhr = new XMLHttpRequest()
            let url = "/settings"
            xhr.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {

                    // Ação típica ocorrida quando o documento está pronto:
                    if(xhr.responseText != null){
                        console.log(xhr.responseText)
                        alert("Option registered, wait for the clock to update!")
                    }
                }
            }
            xhr.open("POST", url, true);
            xhr.send(JSON.stringify(data));
        }


        // Função do botão Reset
        function resetFunction() {

            // Reseta dados da rede
            document.getElementById("ssid").value = ""
            document.getElementById("password").value = ""


            // Reseta dado do modo de exibição do horário
            timeFormat[1].checked = true


            // Reseta País e fuso
            document.getElementById('Country').value = "UTC"
            document.querySelectorAll('[selectable]').forEach(country => {

            country.style.display = 'none'
            if (country.id == document.getElementById("Country").value) {
                country.value = "Etc/GMT"
                country.style.display = 'block'
            }       
            })

            // Esconde timezone não usada e mostra a do reset
            document.querySelectorAll('[selectable]').forEach(country => {
                country.style.display = 'none'
                if (country.id == document.getElementById("Country").value) {
                    country.style.display = 'block'
                }       
            })


            // Reseta o deslocamento do tempo
            moveTime.checked = false
            document.getElementById("hour-move").value = 0
            document.getElementById("min-move").value = 0
            document.getElementById("sec-move").value = 0
            moveTimeDelayAntecipate[0].checked = true
            document.getElementById('show-movetime').style.display = 'none'
            
            
            

            // Reseta o ajuste manual
            turnOffAutoSet.checked = false
            document.getElementById("time").value = ""
            noon[0].checked = true
            document.getElementById("date").value = ""
            document.getElementById('show-turned-off-auto-set').style.display = 'none'
            document.getElementById('noon').style.display = 'none'


            // Reseta Servidor NTP
            document.getElementById('ntp').value = "africa.pool.ntp.org"


            // Reseta dado do modo de exibição do formato da data
            dateType[0].checked = true


            // Altera o resetControl
            if (tempResetControl == 0) {
                resetControl = 1
            } else {
                resetControl = 0
            }
        

            // Salva o Reset
            saveFunction()
        }

        var counter = 2;
        var timer = setInterval(function() {
        
        
            function ajax(config) {
            const xhr = new XMLHttpRequest()
            xhr.open(config.method, config.url, true)

            xhr.onload = e => {
                if (xhr.status === 200) {
                    config.success(xhr.response)
                } else if (xhr.status >= 400) {
                    config.notSuccess({
                        code: xhr.status,
                        text: xhr.statusText
                    })
                }
            }

            xhr.send()
            }  

            ajax({
                url: "/clock",
                method: "get",
                success(answer) {
                    let dateAndTime = answer
                    dateAndTime = answer.split("#")
                    
                    if(dateAndTime[0] != "" && dateAndTime[1] != "") {
                        clock.innerText = dateAndTime[0]
                        calendar.innerText = dateAndTime[1]
                    } else {
                        clock.innerText = "Page Clock" 
                        calendar.innerText = "Waiting For Server"
                    }
                },
                notSuccess(e) {
                    const msg = document.createTextNode(`${e.code}: ${e.text}`)
                    console.log(msg)
                    clock.innerText = "Page Clock" 
                    calendar.innerText = "Waiting For Server"
                }
            })
        
        
        }, 1000);

    </script>
</html>
<span style="display: none;">"</span>
)=====";


// Informações da rede a ser conectada
char ssid[20] = "";
char password[20] = "";

const char* remote_host_google = "www.google.com";
const char* remote_host_wta = "http://worldtimeapi.org/";


int dayOfWeek;
 

// Fuso Horário de São Paulo, horário de Brasília 
// float timeZone = -3;
// int offset = timeZone*3600


// Inicia objecto para uso de UDP
WiFiUDP ntpUDP;


// Váriaveis do cliente NTP 
char ntpServer[30];                              // Servidor ntp --> É obtido quando os dados com JSONObject
int offset = 0;                                  // Deslocamento do fuso horário exemplo São Paulo (UTC - 3:00) -> -3 * 3600

char partialCountryOffsetLink[35];               //  Receberá o link para pegar o offset


// Define cliente NTP Client para pegar tempo
// Objeto responsável por recuperar dados sobre horário
NTPClient timeClient(
    ntpUDP,           //Socket UDP
    ntpServer,        //NTP Server
    offset,           //Deslocamento do horário em relacão ao GMT 0
    60000);     

// Nomes dos dias da semana
char* dayOfWeekNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// Variáveis para salvar data e horário
String formattedDate;
String dayStamp;
String timeStamp;


// Variáveis do LCD
const int RS = D3, EN = D4, d4 = D5, d5 = D6, d6 = D7, d7 = D8;
LiquidCrystal lcd(RS,EN,d4,d5,d6,d7);


// Objeto do módulo RTC
RTC_DS3231 rtc;


// Váriaveis de data e horário
int year;
int month;
int day;
int hour;
int minute;
int second;

int nowYear;
int nowMonth;
int nowDay;
int nowHour;
int nowMinute;
int nowSecond;

// Váriaveis de ajuste manual de hora e data
bool useManualTimeDate;
int manualYear;
int manualMonth;
int manualDay;
int manualHour;
int manualMinute;
int manualSecond;
bool currentUseTimeDate;          // Estas duas váriaveis são usadas para verificar se foi alterado o mode de ajuste manual
bool firstReadTimeDate = true;    //


// Váriaveis de controle do reset
bool resetControl;
bool currentResetControl;
bool firstResetRead= true;

// Variáveis do Timer de 15 Minutos
unsigned long currentMillis  = 0;                // Armazena o instante atual do processamento
unsigned long previousMillis = 0;                // Armazena o tempo anterior do processamento
const long requestInterval15Min = 900000;        // intervalo de 15 minutos em milisegundos
bool passed15min = false;                        // Passou 15 minutos

// Váriaveis do Timer de uma hora
int counterOneHour = 0;                          // Contador da função
bool passedOneHour = false;                      // Passou uma hora


// Váriaveis de deslocamento de tempo
bool useMoveTime;
int moveHour;
int moveMin;
int moveSec;
bool moveSignal;

// Váriavel do formato do horário
int timeFormat;

// Váriavel do formato da Data
int dateType;

// Strings de Data e Hora
String lcdTime = "";                               // Horário a ser impresso no lcd
String lcdDate = "";                               // Data a ser impressa no lcd
String pageTime = "";                              // Horário a ser impresso na página de configuração
String pageDate = "";                              // Data a ser impressa na página de configuração


void startSerial() {
  // Inicialização da Comunicação Serial
  Serial.begin(115200);
  Serial.println("Serial Started!");  
}


void startSPIFFS() {
  // Inicialização do Sistema de Arquivos
  SPIFFS.begin();
  Serial.println("File System Started!");
}


void startLCD() {
  //Incialização do LCD
  lcd.begin(16,2);
  lcd.clear();
  Serial.println("LCD Started!");
}


void startI2C() {
  // Inicialização da Comunicação I2C
  Wire.begin(D1,D2);
  Serial.println("I2C Started!");
}


void startRTC() {
  // Inicializa o Módulo RTC
  bool started = 0;
  started = rtc.begin();
  if (started)
    Serial.println("RTC DS3231 Module Started!");
  else
    Serial.println("RTC DS3231 Module NOT Started!");
}

void startNTP() {
  // Para o Cliente NTP
  // O servidor NTP (netpServer) é confgigurado quando os dados são carregados

  // Inicialização do Cliente NTP para obter Tempo Universal Coordenado
  timeClient.begin();
  Serial.println("NTP Client Started!");
}


void startServer() {
  server.on("/",[](){server.send_P(200,"text/html", webpage);});
  server.on("/settings", HTTP_POST, handleSettingsUpdate);
  server.on("/load", transferDataEspToPage);
  server.on("/clock", transferDateAndTime);

  server.begin();
}


void handleSettingsUpdate()
{
  String data = server.arg("plain");
  DynamicJsonBuffer jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);

  File configFile = SPIFFS.open("/config.json", "w");
  jObject.printTo(configFile);  
  configFile.close();

  firstGetOffset = true; // Caso seja salvo um novo dado força a busca do offset
  manualAjustRtcSeted = false; // Destrava o set do RTC no ajuste manual caso seja ativado o botão save
  
  server.send(200, "application/json", "{\"status\" : \"ok\"}");
  delay(500);
  
  nextState = wifiConnectState;
}


void transferDataEspToPage()
{
  if(SPIFFS.exists("/config.json")){ 
    const char  * _ssid = "", 
                * _pass = "", 
                * _displayTimeFormat, 
                * _timeCountry,                 
                * _timeCountryPartialLink, 
                * _moveTime, 
                * _moveHour, 
                * _moveMin, 
                * _moveSec, 
                * _moveSignal, 
                * _manualAjust, 
                * _noonMode,
                * _ntpServer,
                * _dateType,
                * _resetControl;

    String configParameters = "";
                 
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();
      
      DynamicJsonBuffer jsonBuffer;
      JsonObject& jObject = jsonBuffer.parseObject(buf.get());
      if(jObject.success())
      {
        _ssid = jObject["ssid"];
        _pass = jObject["password"];
        _displayTimeFormat = jObject["displayTimeFormat"];
        _timeCountry = jObject["timeCountry"]; 
        _timeCountryPartialLink = jObject["timeCountryPartialLink"];
        _moveTime = jObject["moveTime"];                
        _moveHour = jObject["moveHour"];
        _moveMin = jObject["moveMin"];
        _moveSec = jObject["moveSec"];
        _moveSignal = jObject["moveSignal"];
        _manualAjust = jObject["manualAjust"];
        _noonMode = jObject["noonMode"];
        _ntpServer = jObject["ntpServer"];
        _dateType = jObject["dateType"];
        _resetControl = jObject["resetControl"];
       
        
        configParameters.concat(_ssid);
        configParameters.concat(",");
        configParameters.concat(_pass);
        configParameters.concat(",");
        configParameters.concat(_displayTimeFormat);
        configParameters.concat(",");
        configParameters.concat(_timeCountry);
        configParameters.concat(",");
        configParameters.concat(_timeCountryPartialLink);
        configParameters.concat(",");
        configParameters.concat(_moveTime);
        configParameters.concat(",");
        configParameters.concat(_moveHour);
        configParameters.concat(",");
        configParameters.concat(_moveMin);
        configParameters.concat(",");
        configParameters.concat(_moveSec);
        configParameters.concat(",");
        configParameters.concat(_moveSignal);
        configParameters.concat(",");
        configParameters.concat(_manualAjust);
        configParameters.concat(",");
        configParameters.concat(_noonMode);
        configParameters.concat(",");
        configParameters.concat(_ntpServer);
        configParameters.concat(",");
        configParameters.concat(_dateType);
        configParameters.concat(",");
        configParameters.concat(_resetControl);
        
        
        Serial.println(configParameters);
        server.send(200,"plain/text",configParameters);
      }
    }
  } else {
      Serial.println("reset");
      server.send(200,"plain/text","reset");
  }

  
}

void transferDateAndTime() {

  String dateAndTime = "";
  dateAndTime.concat(pageTime);
  dateAndTime.concat("#");
  dateAndTime.concat(pageDate);

  server.send(200,"plain/text",dateAndTime);
}


bool past15Min() {
  if (currentMillis - previousMillis >= requestInterval15Min) {
    previousMillis = currentMillis;
    return 1;
  } else {
    return 0;
  }
}


// Função contadora de uma hora
bool pastOneHour(bool passed15min) {
  if (passed15min) {
    counterOneHour++;
  }

  if (counterOneHour == 4) {
    counterOneHour = 0;
    return true;
  } else {
    return false;
  }
}


int stateWifiConnect() {
  // Reseta a Rede
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();          
  delay(1000);

  // Checa se a credencias armazenadas
  if(loadData()) {
    nextState =  stacionModeState;
  } else {
    nextState = accessPointModeState;
  }

}


bool loadData() {
    if(SPIFFS.exists("/config.json")){
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();

      DynamicJsonBuffer jsonBuffer;
      JsonObject& jObject = jsonBuffer.parseObject(buf.get());
     
      const char  * _ssid = "",
                  * _pass = "", 
                  * _displayTimeFormat, 
                  * _timeCountry, 
                  * _timeCountryName, 
                  * _timeCountryLink, 
                  * _timeCountryPartialLink, 
                  * _moveTime, 
                  * _moveHour, 
                  * _moveMin, 
                  * _moveSec, 
                  * _moveSignal, 
                  * _manualAjust, 
                  * _noonMode, 
                  * _manualHour, 
                  * _manualMin, 
                  * _manualSec, 
                  * _manualDay, 
                  * _manualMonth, 
                  * _manualYear,
                  * _ntpServer,
                  * _dateType,
                  * _resetControl;

                  

     if(jObject.success()) {

        _ssid = jObject["ssid"];
        _pass = jObject["password"];
        _displayTimeFormat = jObject["displayTimeFormat"];
        _timeCountry = jObject["timeCountry"]; 
        _timeCountryName = jObject["timeCountryNamed"];
        _timeCountryLink = jObject["timeCountryLink"];
        _timeCountryPartialLink = jObject["timeCountryPartialLink"];
        _moveTime = jObject["moveTime"];                
        _moveHour = jObject["moveHour"];
        _moveMin = jObject["moveMin"];
        _moveSec = jObject["moveSec"];
        _moveSignal = jObject["moveSignal"];
        _manualAjust = jObject["manualAjust"];
        _noonMode = jObject["noonMode"];
        _manualHour = jObject["manualHour"];
        _manualMin = jObject["manualMin"];
        _manualSec = jObject["manualSec"];
        _manualDay = jObject["manualDay"];
        _manualMonth = jObject["manualMonth"];
        _manualYear = jObject["manualYear"];
        _ntpServer = jObject["ntpServer"];
        _dateType = jObject["dateType"];
        _resetControl = jObject["resetControl"];

        // Passagem dos valores JSON para váriaveis do microcontrolador

        // Dados da rede
        strcpy(ssid,_ssid);
        strcpy(password,_pass);

        // Dados do display
        timeFormat = atoi(_displayTimeFormat);
        dateType = atoi(_dateType);

        // Dado do link para obter offset
        strcpy(partialCountryOffsetLink,_timeCountryPartialLink);

        // Dados do deslocamento de tempo
        useMoveTime = (_moveTime[0] =='f') ? 0 : 1;
        moveSignal = atoi(_moveSignal);
        moveHour = atoi(_moveHour);
        moveMin = atoi(_moveMin);
        moveSec = atoi(_moveSec);

        // Dados do ajuste manual de hora e data
        useManualTimeDate = (_manualAjust[0] =='f') ? 0 : 1;
        manualYear = atoi(_manualYear);
        manualMonth = atoi(_manualMonth);
        manualDay = atoi(_manualDay);
        manualHour = atoi(_manualHour);
        manualMinute = atoi(_manualMin);
        manualSecond = atoi(_manualSec);

        // Controle do reset
        resetControl = atoi(_resetControl);
        
        //------------ Verifica se houve alteração de ajuste manual para automático e vice versa
        if (firstReadTimeDate) {
          currentUseTimeDate = useManualTimeDate;
          firstReadTimeDate = false;
        }

        if (useManualTimeDate != currentUseTimeDate) {
          currentUseTimeDate = useManualTimeDate;
          if (!useManualTimeDate) {
            readNTP();
          }
          
        }
        //------------------------------------



        //---------------------- Verifica se o reset foi acionado
        if (firstResetRead) {
          currentResetControl = resetControl;
          firstResetRead = false;
        }

        if (resetControl != currentResetControl) { // Se acionado reseta os parâmetros abaixo
          currentResetControl = resetControl;
          nextState = wifiConnectState;
          firstNTPRead = true;
          rtcSeted = false;
          pageTime = "";                             
          pageDate = ""; 

        }
        //------------------------------------

        // Dado do servidor NTP
        strcpy(ntpServer,_ntpServer);
        
        return true;

      } else {

        return false;
        
      }
    }
  } else {

    return false;

  }
}


int stateStacionMode() {

  if (startStacionMode()) {
    Serial.println("Stacion Mode Success!");
    WiFi.printDiag(Serial);

    if(!Ping.ping(remote_host_google) && rtcSeted) { //verifica se há conexão com a internet
      return getRTCState;
    }

    return getOffsetState;

  } else {

    return accessPointModeState;

  }
}


bool startStacionMode() {
  // Seta Stacion Mode
  WiFi.mode(WIFI_STA);

  // Conecta a Rede Wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (!rtcSeted) {
    lcd.clear();
    lcd.print("Connecting");
  }
  
  WiFi.begin(ssid, password);

  // Marca inicio do timer de conexão
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (!rtcSeted) {
      lcd.print(".");
    }

    if ((unsigned long) (millis() - startTime >= 5000)) {
      break;
    }

  }


   if (WiFi.status() == WL_CONNECTED) {

    // Imnprime Endereço Local de IP e Inicia o Servidor Web
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.print("IP address:");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    delay(6000);
    lcd.clear();

    return true;

  } else {

    return false;

  }
  
}


int stateAccessPointMode() {
  
  WiFi.mode(WIFI_AP);   // Seta Access Point Mode
  WiFi.softAPConfig(local_ip, gateway, netmask);
  WiFi.softAP(clockSsid, clockPassword);
  WiFi.printDiag(Serial);

  if (passedOneHour) {
    return wifiConnectState;
  }

  if (rtcSeted) {
    return getRTCState;
  }

  lcd.clear();
  lcd.print("   AutoClock   ");
  lcd.setCursor(0,1);
  lcd.print("  192.168.0.1  ");

  return stopState;
}

int stateGetOffset() {
  if (firstGetOffset) {
    getOffset();
    firstGetOffset = false;
  }

  if (WiFi.status() == WL_CONNECTED)
    getOffset();

    return NTPReadState;
}

void getOffset() {

  String countryOffsetLink = "";
  countryOffsetLink.concat("http://worldtimeapi.org/api/timezone/");
  countryOffsetLink.concat(partialCountryOffsetLink);

  /*
    Funcionamento do algoritimo
      1- Faz uma requisição http get para api http://worldtimeapi.org/api/timezone/<timezone>
      2- Recebe um obejto --> {"week_number":8,"utc_offset":"+01:00","utc_datetime":"2020-02-23T09:12:33.97821.......... e assim por diante
      3- Numero String        0123456789....
      4- Esse objeto é armazenado em um string
      5- O primeiro u está na posição 8 e o segundo já é o utc
      6- após o primeiro u ou seja a partir da posição 9 procura o primeiro u
      7- A partir da posição do 'u' do utc ele indetifica as posições da string do offset (+01:00)
      8- Com a posições em mão convertemos o sinal para +1 ou -1 dependendo do caracter
      9- A partir da tabela ascii convertemos os caracteres de hora e min em números
      10- calculamos o offset --> fórmula --> "sinal *  ((horas*3600)+(min*60))"
  */

  //----------------Pegando o Site ----------------------------
  //Cria o cliente HTTP
  HTTPClient http;
  
  //Inicializa o cliente HTTP no site do dado
  http.begin(countryOffsetLink);

  //Pega o UTC
  int httpCode = http.GET();
  if (httpCode > 0) { //Check the returning code
 
      String payload = http.getString();              //Pega a requisição(paǵina) como string

      // Começo e fim da string do offset
      int offsetStart;
      int offsetEnd;

      // Dados numéricos do offset
      int offsetSignal;
      int offsetHour;
      int offsetMin;
      int offsetValue;

      // Calcula o inicio e o fim da string do offset
      for (int i = 9; i < 50; i++) {
        if (payload[i] == 'u') {
          Serial.println(i);
          offsetStart = i + 13;
          offsetEnd = i + 13 + 6;
          break;
       }
      }

      // Imprime a tring do offset
      for (int i = offsetStart; i < offsetEnd; i++) {
        Serial.print(payload[i]);
      }
      Serial.println("");

      // Calcula o sinal do offset
      if(payload[offsetStart] == '+') {
        offsetSignal = 1;
      } else if (payload[offsetStart] == '-') {
        offsetSignal = -1;
      }

      // Converte a string do offset para números
      offsetHour = ((payload[offsetStart + 1] - 48) * 10) + (payload[offsetStart + 2] - 48);
      offsetMin = ((payload[offsetStart + 4] - 48) * 10) + (payload[offsetStart + 5] - 48);
      offsetValue = (offsetHour * 3600) + (offsetMin * 60);
      offsetValue *= offsetSignal;

      offset = offsetValue; // Passa para o sistema o valor do offset calculado
  }
 
  http.end();   // Encerra conexão

}

int stateNTPRead() {
  if (firstNTPRead) {
    readNTP();
    firstNTPRead = false;  
  }

  if (WiFi.status() == WL_CONNECTED)
    readNTP();
    
  return setRTCState;
}

void readNTP() {

  timeClient.setTimeOffset(offset);            // Atualiza o offset para usar o calculado pelo getOffset()

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  
  // O formattedDate tem o seguinte formato:
  // 2018-05-28T16:00:13Z
  // Precisamos extrair data e horario
  
  formattedDate = timeClient.getFormattedDate(); // Pega data no formato comentado acima

  dayOfWeek = timeClient.getDay();               // Pega o numero do dia da semana para ser usado no vetor dayOfWeekNames[]

  // Extraindo data
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  
  // Extraindo horario
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  splitDayAndTimeStamps();                      // Converte caracteres das strings "dayStamp" e "timeStamp" para números inteiros
}

void splitDayAndTimeStamps() {
  // Converte caracteres das strings "dayStamp" e "timeStamp" para números inteiros
  year = ((dayStamp[2] - 48) * 10) + (dayStamp[3] - 48);
  month = ((dayStamp[5] - 48) * 10) + (dayStamp[6] - 48);
  day = ((dayStamp[8] - 48) * 10) + (dayStamp[9] - 48);
  hour = ((timeStamp[0] - 48) * 10) + (timeStamp[1] - 48);
  minute = ((timeStamp[3] - 48) * 10) + (timeStamp[4] - 48);
  second = ((timeStamp[6] - 48) * 10) + (timeStamp[7] - 48);
}


int stateSetRTC() {

  if (useManualTimeDate) {
    rtc.adjust(DateTime(manualYear,manualMonth,manualDay,manualHour,manualMinute,manualSecond)); // Ajusta o Horário no módulo RTC no modo manual
  } else { 
    if (!manualAjustRtcSeted) {
      rtc.adjust(DateTime(year,month,day,hour,minute,second)); // Ajusta o Horário no módulo RTC no modo automático 
      manualAjustRtcSeted = true;     
    }
  }
  
  rtcSeted = true;
  return getRTCState;
}


int stateGetRTC() {
  DateTime nowDateTime = rtc.now();
  DateTime pastTime = (nowDateTime - TimeSpan(0,moveHour,moveMin,moveSec));   // (dias,horas,minutos,segundos)
  DateTime futureTime = (nowDateTime + TimeSpan(0,moveHour,moveMin,moveSec));   // (dias,horas,minutos,segundos)
  
  if (useMoveTime) {
    if (moveSignal) {
      dayOfWeek = pastTime.dayOfTheWeek();
    
      nowYear = pastTime.year();
      nowMonth = pastTime.month();
      nowDay = pastTime.day();
      nowHour = pastTime.hour();
      nowMinute = pastTime.minute();
      nowSecond = pastTime.second(); 
    } else {
      dayOfWeek = futureTime.dayOfTheWeek();
    
      nowYear = futureTime.year();
      nowMonth = futureTime.month();
      nowDay = futureTime.day();
      nowHour = futureTime.hour();
      nowMinute = futureTime.minute();
      nowSecond = futureTime.second(); 
    }
      
  } else {
    dayOfWeek = nowDateTime.dayOfTheWeek();
  
    nowYear = nowDateTime.year();
    nowMonth = nowDateTime.month();
    nowDay = nowDateTime.day();
    nowHour = nowDateTime.hour();
    nowMinute = nowDateTime.minute();
    nowSecond = nowDateTime.second(); 
  }

  return displayTimeState;
}


int stateDisplayTime() {
  
  stringGenerator();
  
  // lcd.clear();

  // Imprime o horário no display
  lcd.setCursor(0,0);
  lcd.print(lcdTime);

  // Imprime a Data no display
  lcd.setCursor(0,1);
  lcd.print(lcdDate);

  

  return idle1SecState;
}

void stringGenerator() {
  
    // Reseta strings
    lcdTime = "";                               
    lcdDate = "";                              
    pageTime = "";                              
    pageDate = ""; 

    //Monta String para horário
    if (timeFormat == 12) {       // Monta strings de tempo para formato 12 horas
        bool pm = false;
        int tempTime;

        // Campo Hora
        if (nowHour < 10) {
            pageTime.concat("0");
            pageTime.concat(nowHour);
        } else {
            if (nowHour > 12) {
                pm = true;
                tempTime = nowHour - 12;
                if (tempTime < 10) {
                    pageTime.concat("0");
                }
                pageTime.concat(tempTime);
            } else {
                pageTime.concat(nowHour);
            }
        }

        pageTime.concat(":");

        // Campo minuto
        if (nowMinute < 10) {
            pageTime.concat("0");
            pageTime.concat(nowMinute);
        } else {
            pageTime.concat(nowMinute);
        }

        pageTime.concat(":");

        // Campo segundo
        if (nowSecond < 10) {
            pageTime.concat("0");
            pageTime.concat(nowSecond);
        } else {
            pageTime.concat(nowSecond);
        }

        // Campo AM ou PM
        if (pm) {
            pageTime.concat(" PM");
        } else {
            pageTime.concat(" AM");
        }
    
        lcdTime.concat("  ");
        lcdTime.concat(pageTime);
        lcdTime.concat("  ");
    
    } else {        // Monta String para 24 horas

        // Campo Hora
        if (nowHour < 10) {
            pageTime.concat("0");
            pageTime.concat(nowHour);
        } else {
            pageTime.concat(nowHour);
        }

        pageTime.concat(":");

        // Campo minuto
        if (nowMinute < 10) {
            pageTime.concat("0");
            pageTime.concat(nowMinute);
        } else {
            pageTime.concat(nowMinute);
        }

        pageTime.concat(":");

        // Campo segundo
        if (nowSecond < 10) {
            pageTime.concat("0");
            pageTime.concat(nowSecond);
        } else {
            pageTime.concat(nowSecond);
        }

        lcdTime.concat("    ");
        lcdTime.concat(pageTime);
        lcdTime.concat("   ");
    
    }

  // Monta string para data

    lcdDate.concat("   ");
    pageDate.concat(dayOfWeekNames[dayOfWeek]);
  
    if (dateType == 0) {         // DD/MM/AAAA

        pageDate.concat(" - ");

        if (nowDay < 10) {
            lcdDate.concat("0");
            pageDate.concat("0");
            lcdDate.concat(nowDay);
            pageDate.concat(nowDay);
        } else {
            lcdDate.concat(nowDay);
            pageDate.concat(nowDay);
        }
    
            lcdDate.concat("/");
            pageDate.concat("/");
    
        if (nowMonth < 10) {
            lcdDate.concat("0");
            pageDate.concat("0");
            lcdDate.concat(nowMonth);
            pageDate.concat(nowMonth);
        } else {
            lcdDate.concat(nowMonth);
            pageDate.concat(nowMonth);
        }
  
        lcdDate.concat("/");
        pageDate.concat("/");

        lcdDate.concat(nowYear);
        pageDate.concat(nowYear);

        lcdDate.concat("  ");
    
    
  } else if (dateType == 1) {  // MM/DD/AAAA
        
        pageDate.concat(" - ");

        if (nowMonth < 10) {
            lcdDate.concat("0");
            pageDate.concat("0");
            lcdDate.concat(nowMonth);
            pageDate.concat(nowMonth);
        } else {
            lcdDate.concat(nowMonth);
            pageDate.concat(nowMonth);
        }

            lcdDate.concat("/");
            pageDate.concat("/");

        if (nowDay < 10) {
            lcdDate.concat("0");
            pageDate.concat("0");
            lcdDate.concat(nowDay);
            pageDate.concat(nowDay);
        } else {
            lcdDate.concat(nowDay);
            pageDate.concat(nowDay);
        }

        lcdDate.concat("/");
        pageDate.concat("/");

        lcdDate.concat(nowYear);
        pageDate.concat(nowYear);

        lcdDate.concat("  ");
        
    } else {                       // AAAA-MM-DD
        pageDate.concat(", ");

        lcdDate.concat(nowYear);
        pageDate.concat(nowYear);

        lcdDate.concat("-");
        pageDate.concat("-");

        if (nowMonth < 10) {
            lcdDate.concat("0");
            pageDate.concat("0");
            lcdDate.concat(nowMonth);
            pageDate.concat(nowMonth);
        } else {
            lcdDate.concat(nowMonth);
            pageDate.concat(nowMonth);
        }

        lcdDate.concat("-");
        pageDate.concat("-");

        if (nowDay < 10) {
            lcdDate.concat("0");
            pageDate.concat("0");
            lcdDate.concat(nowDay);
            pageDate.concat(nowDay);
        } else {
            lcdDate.concat(nowDay);
            pageDate.concat(nowDay);
        }

        lcdDate.concat("  ");
    
  }
}


void setup() {

  startSerial();  // Inicia comunicação serial
  startSPIFFS();  // Inicia o sistema de arquivos
  startLCD();     // Inicia modulo display LCD
  startI2C();     // Inicializa Comunicação I2C
  startRTC();     // Inicializa o Módulo Real Time Clock
  startNTP();     // Inicializa Cliente NTP
  startServer();  // Inicializa o Servidor

}



void loop() {

  currentMillis = millis();
  passed15min = past15Min();
  passedOneHour = pastOneHour(passed15min);
  
  if (passed15min && (WiFi.status() == WL_CONNECTED) && !useManualTimeDate) {

     if(Ping.ping(remote_host_wta)) {  // Verifica se há conexão com a internet e verifica se a api de offset está online
       nextState = getOffsetState;
     } else {
       nextState = getRTCState;
     }
        
  }

  if (passedOneHour && (WiFi.status() != WL_CONNECTED)) {
    nextState = wifiConnectState;
  }
  
  currentState = nextState;

  switch (currentState) {
    
    case wifiConnectState:
      nextState = stateWifiConnect();
      break;
    case stacionModeState:
      nextState = stateStacionMode();
      break;
    case accessPointModeState:
      nextState = stateAccessPointMode();
      break;
    case getOffsetState:
      nextState = stateGetOffset();
      break;
    case NTPReadState:
      nextState = stateNTPRead();
      break;
    case setRTCState:
      nextState = stateSetRTC();
      break;
    case getRTCState:
      nextState = stateGetRTC();
      break;
    case displayTimeState:
      nextState = stateDisplayTime();
      break;
    case idle1SecState:
      delay (1000);
      nextState = getRTCState;
      break;
    case stopState:
      break;
  }

  server.handleClient();
}
