<?php

$device = ""; 
$user = "";
$state = "";
$color = "";

//Recupero il valore del parametro "box"
if (isset($_GET['box'])) { 
    $device = $_GET['box']; 
}  
//Recupero il valore del parametro "user"
if (isset($_GET['user'])) { 
    $user = $_GET['user'];
}  
//Recupero il valore del parametro "act"
if (isset($_GET['act'])) { 
    $state = $_GET['act']; 
}
//Recupero il valore del parametro "color"
if (isset($_GET['color'])) { 
    $color = $_GET['color'];
}  
//eseguo la connessione al database sul server locale
//inserendo nome utente e password

$conn  = new mysqli('localhost', 'arduino', 'arduino', 'arduino');
 
//gestione degli errori
// Check connection
if ($conn ->connect_error) {
	die("Connection failed: " . $conn->connect_error);
	echo "connection KO";
	$state = "N";
}

/* echo $device . "<br>";
echo $user . "<br>";
echo $state . "<br>";
echo $color . "<br>"; */
if ($color == 1) {
	$sql = "SELECT Stat FROM `arduino`.`stat` WHERE Box='". $device ."'";
	$rs=$conn->query($sql);
 
	if($rs === false) {
		trigger_error('Wrong SQL: ' . $sql . ' Error: ' . $conn->error, E_USER_ERROR);
	} else {
		$arr = $rs->fetch_all(MYSQLI_ASSOC);
	}
	foreach($arr as $row) {
//		echo "Stat: " . $row['Stat'] . "<br>";
		if ($state != "N") $state = $row['Stat'];
	}
} else {	 
	//creo una stringa sql di inserimento con i valori recuperati dall'url
	$sql = "INSERT INTO `arduino`.`rfid` (`ID` , `Device_ID` , `User_ID` , `State_ID` )
	VALUES (NULL ,  '" . $device . "', '" . $user . "', '" . $state . "' );";
	//eseguo la query

	if ($conn->query($sql) === TRUE) {
	} else {
	echo "query insert KO";		
	$state="N";
	}

	$sql = ("UPDATE `arduino`.`stat` SET Stat=".$state." WHERE Box='".$device."'");

	if ($conn->query($sql) === TRUE) {
	} else {
		echo "query update KO";
		$state="N";
	}
}
$conn->close();
echo "#" . $state . "@" . "<br>";
?>