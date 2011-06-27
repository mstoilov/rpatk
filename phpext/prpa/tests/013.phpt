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

START rid: 4, Section: [PersonName]
FirstName = John
LastName = Smith


START rid: 1, SectionName: PersonName
END rid: 1, SectionName: PersonName
START rid: 2, Name: FirstName
END rid: 2, Name: FirstName
START rid: 3, Value: John
END rid: 3, Value: John
START rid: 2, Name: LastName
END rid: 2, Name: LastName
START rid: 3, Value: Smith
END rid: 3, Value: Smith
END rid: 4, Section: [PersonName]
FirstName = John
LastName = Smith


START rid: 4, Section: [PersonAddress]
Street = NE 231
; this is a comment
City = Seattle
State = Washington
ZIP = 98115

START rid: 1, SectionName: PersonAddress
END rid: 1, SectionName: PersonAddress
START rid: 2, Name: Street
END rid: 2, Name: Street
START rid: 3, Value: NE 231
END rid: 3, Value: NE 231
START rid: 2, Name: City
END rid: 2, Name: City
START rid: 3, Value: Seattle
END rid: 3, Value: Seattle
START rid: 2, Name: State
END rid: 2, Name: State
START rid: 3, Value: Washington
END rid: 3, Value: Washington
START rid: 2, Name: ZIP
END rid: 2, Name: ZIP
START rid: 3, Value: 98115
END rid: 3, Value: 98115
END rid: 4, Section: [PersonAddress]
Street = NE 231
; this is a comment
City = Seattle
State = Washington
ZIP = 98115
