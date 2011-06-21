--TEST--
rpaparse($bnf, $input, $records, $error);
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
	$records = 0;
	$error = 0;
	$bnf = "#!emitid first 1\n" .
	"#!emitid last 2\n" .
	"#!emitid name 3\n" .
	"first ::= [a-z]+\n" .
	"last ::= [a-z]+\n" .
	"name ::= <first> ' '+ <last>\n";
	$myname = "John Smith";	
	$ret = rpaparse($bnf, RPA_ENCODING_ICASE_UTF8, $myname, $records, $error);
	if ($ret < 0)
		die($error . "\n");
	$matched = "matched: " . substr($myname, 0, $ret) . "\n";
	echo($matched);
	
	if (is_array($records)) {
		foreach ($records as $record) {
			if ($record['type'] & RPA_RECORD_START)
				echo("START ");
			else if ($record['type'] & RPA_RECORD_END)
				echo("END   ");
			else
				echo("UNKNOWN");
			echo ($record['uid'] . ", " . $record['rule'] . ": " . $record['input'] . "\n");
		}
	}
?>
--EXPECT--
matched: John Smith
START 3, name: John Smith
START 1, first: John
END   1, first: John
START 2, last: Smith
END   2, last: Smith
END   3, name: John Smith
