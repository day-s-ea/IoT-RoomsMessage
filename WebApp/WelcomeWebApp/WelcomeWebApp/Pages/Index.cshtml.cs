using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using MQTTnet;
using MQTTnet.Client.Connecting;
using MQTTnet.Client.Disconnecting;
using MQTTnet.Client.Options;
using MQTTnet.Extensions.ManagedClient;
using Newtonsoft.Json.Linq;

namespace WelcomeWebApp.Pages
{
    public class IndexModel : PageModel
    {
        private readonly ILogger<IndexModel> _logger;
        //Data input
        [BindProperty]
        public int IdRoom { get; set; }

        [BindProperty]
        public string Message { get; set; }

        [BindProperty]
        public int Time { get; set; }


        //Topic where to publish Mqtt messages
        static string topicMqttPublish = "bedroom/msgWelcome";

        //Time how often a message is sent
        //static int timeSendMessage = 1000;

        //Client Mqtt
        static IManagedMqttClient _mqttClient;
        //Every how many seconds it tries to reconnect
        static int timeMqttReconnect = 5;
        //Port number of Mqtt
        static int numberPortMqtt = 1883;
        //Ip of the Mqtt Server
        static string serverMqtt = "broker.hivemq.com";


        public IndexModel(ILogger<IndexModel> logger)
        {
            _logger = logger;
        }

        public void OnGet()
        {

        }
        public void OnPost()
        {
          
            //to have unique names 
            string idNameClientMqtt = Guid.NewGuid().ToString("N");

            #region Opzioni Client MQTT
            // Creates a new client
            MqttClientOptionsBuilder builder = new MqttClientOptionsBuilder()
                                                    .WithClientId("WebApp-" + idNameClientMqtt.ToString())
                                                    .WithTcpServer(serverMqtt, numberPortMqtt);
            //.WithTls()
            //.WithCredentials("WebApp-" + idNameClientMqtt.ToString(), "ConsolePassword000");  for the password

            // Create client options objects
            ManagedMqttClientOptions options = new ManagedMqttClientOptionsBuilder()
                                    .WithAutoReconnectDelay(TimeSpan.FromSeconds(timeMqttReconnect))
                                    .WithClientOptions(builder.Build())
                                    .Build();

            // Creates the client object
            _mqttClient = new MqttFactory().CreateManagedMqttClient();

            // Set up handlers
            _mqttClient.ConnectedHandler = new MqttClientConnectedHandlerDelegate(OnConnected);
            _mqttClient.DisconnectedHandler = new MqttClientDisconnectedHandlerDelegate(OnDisconnected);
            _mqttClient.ConnectingFailedHandler = new ConnectingFailedHandlerDelegate(OnConnectingFailed);

            // Starts a connection with the Broker
            _mqttClient.StartAsync(options).GetAwaiter().GetResult();
            #endregion Opzioni Client MQTT

            while (_mqttClient.IsConnected == false) { }

            //Creation dict to be transformed into json
            Dictionary<string, object> objToSend = new Dictionary<string, object>();

            //json elements
            objToSend.Add("idGateway", 1);
            objToSend.Add("idRoom", IdRoom);
            objToSend.Add("message", Message);
            objToSend.Add("time", Time);

            //JObject creation from Dict
            var jobjToConvert = JObject.FromObject(objToSend);
            //Conversion from JObject to Json String
            string jsonStringToSend = jobjToConvert.ToString().Replace("\r\n", "");

            //Creating the Mqtt message
            var message = new MqttApplicationMessageBuilder()
           .WithTopic(topicMqttPublish)
           .WithPayload(jsonStringToSend)
           .WithAtMostOnceQoS()
           .WithRetainFlag(true) //the last message
           .Build();

            //Publish the message in the Mqtt Broker
            _mqttClient.PublishAsync(message);
        }

        public static void OnConnected(MqttClientConnectedEventArgs obj)
        {
            Console.WriteLine("Successfully connected.");
        }

        public static void OnConnectingFailed(ManagedProcessFailedEventArgs obj)
        {
            Console.WriteLine("Couldn't connect to broker.");
        }

        public static void OnDisconnected(MqttClientDisconnectedEventArgs obj)
        {
            Console.WriteLine("Successfully disconnected.");
        }
    }
}