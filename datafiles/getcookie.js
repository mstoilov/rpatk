function setCookie(c_name,value,expiredays)
{
	var exdate=new Date();
	exdate.setDate(exdate.getDate()+expiredays);
	document.cookie=c_name+ "=" +escape(value)+((expiredays==null) ? "" : "; expires="+exdate.toGMTString());
}


function getCookie(c_name)
{
	if (document.cookie.length>0)
	{
		c_start=document.cookie.indexOf(c_name + "=");
		if (c_start!=-1)
		{ 
			c_start=c_start + c_name.length+1 ;
			c_end=document.cookie.indexOf(";",c_start);
			if (c_end==-1) 
				c_end=document.cookie.length;
			return unescape(document.cookie.substring(c_start,c_end));
		} 
	}
	return "";
}


function checkCookie()
{
	username=getCookie('username');
	if (username!=null && username!="")
	{
		alert('Welcome again '+username+'!');
	}
	else 
	{
		username=prompt('Please enter your name:',"");
		if (username!=null && username!="")
		{
			setCookie('username',username,365);
		}
	}
}


//You will receive a different greeting based
//on what day it is. Note that Sunday=0,
//Monday=1, Tuesday=2, etc.

var d=new Date();
theDay=d.getDay();
switch (theDay)
{
case 5:
	document.write("Finally Friday");
	break;
case 6:
	document.write("Super Saturday");
	break;
case 0:
	document.write("Sleepy Sunday");
	break;
default:
	document.write("I'm looking forward to this weekend!");
}


function testSwitch ()
{
/*	You will receive a different greeting based
 *	on what day it is. Note that Sunday=0,
 *	Monday=1, Tuesday=2, etc.
 */

	var d=new Date();
	theDay=d.getDay();
	switch (theDay)
	{
	case 5:
		document.write("Finally Friday");
		break;
	case 6:
		document.write("Super Saturday");
		break;
	case 0:
		document.write("Sleepy Sunday");
		break;
	default:
		document.write("I'm looking forward to this weekend!");
	}
}


function testIfElse(arg1, arg2)
{
//If the time is less than 10, you will get a "Good morning" greeting.
//Otherwise you will get a "Good day" greeting.

	var d = new Date();
	var time = d.getHours();

	if (time < 10)
	{
		document.write("Good morning!");
	}
	else
	{
		document.write("Good day!");
		c = 5 + 2 - d;
	}
}


function testForLoop()
{
	var i=0;
	for (i=0; i<=5; i++, b-=1)
	{
		document.write("The number is " + i);
		document.write("<br />");
	}
}


function testWhileLoop ()
{
	while (i<=5)
	{
		document.write("The number is " + i);
		document.write("<br />");
		i++;
	}
}


function testForBreakLoop ( )
{
	for (i=0;i<=10;i++)
	{
		if (i==3)
		{

			continue;

		}


		document.write("The number is " + i);

		document.write("<br />");
	}


}


function testForInLoop() 
{
	var x;
	var mycars = new Array();
	mycars[0] = "Saab";
	mycars[1] = "Volvo";
	mycars[2] = "BMW";

	for (x in mycars)
	{
		document.write(mycars[x] + "<br />");
	}

}


function testTryCatch()
{
	try
	{
		adddlert("Welcome guest!");
	}

	catch(err)

	{
		txt="There was an error on this page.\n\n";
		txt+="Error description: " + err.description + "\n\n";
		txt+= 'Click OK to continue.\n\n';
		alert(txt);
	}
}


function testTryCatchFinaly ()
{
	try
	{
		adddlert("Welcome guest!");
	}

	catch(err)

	{
		txt="There was an error on this page.\n\n";
		txt+="Error description: " + err.description + "\n\n";
		txt+= 'Click OK to continue.\n\n';
		alert(txt);
	}

	finally

	{
		txt += "nothing";
	}
}


function testThrow()
{
	var x=prompt("Enter a number between 0 and 10:","");
try
{
	if(x>10)
    {
		throw "Err1";
    }
	else if(x<0)
    {
		throw "Err2";
    }
	else if(isNaN(x))
    {
		throw "Err3";
    }
}
catch(er)
{
	if(er === "Err1")
    {
		alert("Error! The value is too high");
    }
	if(er=="Err2")
    {
		alert("Error! The value is too low");
    }
	if(er=="Err3")
    {
		alert("Error! The value is not a number");
    }
}
}
