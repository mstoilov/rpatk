--TEST--
Parse .ini file
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
	$records = 0;
	$error = 0;
	$bnf = "#!emitnone\n" .
	"#!emitid SectionName 1\n" .
	"#!emitid Name 2\n" .
	"#!emitid Value 3\n" .
	"#!emitid Section 4\n" .	
	"S					::= [#0x0009] | [#0x000B] | [#0x000C] | [#0x0020] | [#0x00A0] | [#0xFEFF]\n" .
	"EOL				::= [#0x000D] [#0x000A] | [#0x000A] | [#0x000D] | [#0x2028] | [#0x2029]\n" .
	"SectionName 		::= [a-zA-Z][a-zA-Z0-9]*\n" .
	"SectionLine 		::= '[' <SectionName> ']' <S>* \n" .
	"Comment			::= ';' (. - <EOL>)+ \n" .	
	"Name 				::= [a-zA-Z][a-zA-Z0-9]* \n" .
	"Value 				::= (. - <EOL>)+ \n" .
	"NameValue			::= <Name> <S>* '=' <S>* <Value>? \n" .
	"Section			::= <SectionLine> (<NameValue>|<EOL>|<S>|<Comment>)*\n" .
	"Ini 				::= <Section>+\n";
	

	$inifile = "[PersonName]\n" .
	"FirstName = John\n" .
	"LastName = Smith\n\n" .
	"[PersonAddress]\n" .		
	"Street = NE 231\n" .
	"; this is a comment\n" .
	"City = Seattle\n" .
	"State = Washington\n" .
	"ZIP = 98115\n";


	$ret = rpaparse($bnf, RPA_ENCODING_UTF8, $inifile, $records, $error);
	if ($ret < 0)
		die($error['message'] . "\n");
	echo($inifile . "\n");
	if (is_array($records)) {
		foreach ($records as $record) {
			if ($record['type'] & RPA_RECORD_START) {
				echo ("START rid: " . $record['uid'] . ", " . $record['rule'] . ": " . $record['input'] . "\n");
			} else 	if ($record['type'] & RPA_RECORD_END) {
				echo ("END rid: " . $record['uid'] . ", " . $record['rule'] . ": " . $record['input'] . "\n");
			}
			
		}
	}
?>
--EXPECT--
[PersonName]
FirstName = John
LastName = Smith

[PersonAddress]
Street = NE 231
; this is a comment
City = Seattle
State = Washington
ZIP = 98115

rid: 1, SectionName: PersonName
rid: 2, Name: FirstName
rid: 3, Value: John
rid: 2, Name: LastName
rid: 3, Value: Smith
rid: 1, SectionName: PersonAddress
rid: 2, Name: Street
rid: 3, Value: NE 231
rid: 2, Name: City
rid: 3, Value: Seattle
rid: 2, Name: State
rid: 3, Value: Washington
rid: 2, Name: ZIP
rid: 3, Value: 98115
