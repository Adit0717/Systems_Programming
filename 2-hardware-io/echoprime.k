code echoprime

	function main ()

		var 
			ch: char
			isStarting: bool = true

		while true
			ch = GetCh()
			
			if isStarting && ch == 'q'
				debug
			endIf
			
			printChar(ch)

			if ch == '\n'
                isStarting = true
            else
                isStarting = false
            endIf

		endWhile
	endFunction
endCode