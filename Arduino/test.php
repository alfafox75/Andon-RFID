<?php

//Recupero il valore del parametro "device"
$device = $_GET['device'];
 
//Recupero il valore del parametro "user"
$user = $_GET['user'];

//Recupero il valore del parametro "state"
$state = $_GET['state'];
 
//eseguo la connessione al database sul server locale
//inserendo nome utente e password

$conn  = new mysqli('localhost', 'arduino', 'arduino', 'arduino');
 
//gestione degli errori
// Check connection
if ($conn ->connect_error) {
die("Connection failed: " . $conn->connect_error);
}
echo "100";

 
//creo una stringa sql di inserimento con i valori recuperati dall'url
$sql = "INSERT INTO `arduino`.`rfid` (`ID` , `Device_ID` , `User_ID` , `State_ID` )
VALUES (NULL ,  '" . $device . "', '" . $user . "', '" . $state . "' );";
//eseguo la query

if ($conn->query($sql) === TRUE) {
echo $state;
} else {
echo "Error: " . $sql . "<br>" . $conn->error;
}$conn->close();

?>