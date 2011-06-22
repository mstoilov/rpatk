--TEST--
rpascan($bnf, $input, $where, $error);
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
	$where = 0;
	$error = 0;
	$bnf = "#!emitid first 1\n" .
	"#!emitid last 2\n" .
	"#!emitid name 3\n" .
	"first ::= [a-z]+\n" .
	"last ::= [a-z]+\n" .
	"name ::= <first> ' '+ <last>\n";
	$myname = "     : John Smith";	
	if (($ret = rpascan($bnf, RPA_ENCODING_ICASE_UTF8, $myname, $where, $error)) < 0)
		die($error['message'] . "\n");
	echo("matched: " . substr($myname, $where, $ret) . " @ " . $where . "\n");
?>
--EXPECT--
matched: John Smith @ 7
