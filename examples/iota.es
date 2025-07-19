"0: this should be 0" = iota()
"1: this should be 0" = $/"0: this should be 0"
"2: this should be [1]" = [iota()]
"3: this should be [1]" = $/"2: this should be [1]"
"4: this should be 5" = sum_i(iota(), iota())
"5: this should be 9" = sum_ai([$/"4: this should be 5", iota()])
"6: this should be 9" = $/"5: this should be 9"
"7: this should be 1" = $/"2: this should be [1]"[0]

object = {a = iota(), b = iota(), c = [iota()], d = { e = iota() }}
"8: this should not change the values" = $/object
