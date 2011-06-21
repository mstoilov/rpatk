--TEST--
rpa_dbex_parse($hDbex, $pattern, $myname)
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
	$hDbex = rpa_dbex_create();
	$records = 0;
	rpa_dbex_open($hDbex);

	$bnf = "#!emitid first 1\n" .
	"#!emitid last 2\n" .
	"#!emitid name 3\n" .
	"first ::= [a-z]+\n" .
	"last ::= [a-z]+\n" .
	"name ::= <first> ' '+ <last>\n";
	if (rpa_dbex_load($hDbex, $bnf) < 0) {
		echo("rpa_dbex_load failed.\n");
		echo(rpa_dbex_error($hDbex) . "\n");
	}
	
	rpa_dbex_close($hDbex);
	if (rpa_dbex_compile($hDbex) < 0) {
		die(rpa_dbex_error($hDbex). "\n");	
	}
	$pattern = rpa_dbex_lookup($hDbex, "name");
	if ($pattern == -1) {
		die(rpa_dbex_error($hDbex). "\n");
	}
	$myname = "John Smith";
	$stat = rpa_stat_create($hDbex, 0);
	
	$ret = rpa_stat_parse($stat, $pattern, RPA_ENCODING_ICASE_UTF8, $myname, $records);
	if ($ret < 0) {
		die(rpa_stat_error($stat). "\n");	
	}
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
			echo ("(" . $record['offset'] . ", " . $record['size'] . ") " . $record['uid'] . ", " . $record['rule'] . ": " . $record['input'] . "\n");
		}
	}
	unset($stat);
	unset($hDbex);
      
?>
--EXPECT--
matched: John Smith
START (0, 10) 3, name: John Smith
START (0, 4) 1, first: John
END   (0, 4) 1, first: John
START (5, 5) 2, last: Smith
END   (5, 5) 2, last: Smith
END   (0, 10) 3, name: John Smith
