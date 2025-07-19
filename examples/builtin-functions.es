# `integer sum_i(integer...)`, receive N `integers` as argument and return the sum of all of it as integer.

sum_i = sum_i(10, 30, 50)

# `float sum_f(float...)`, receive N `floats` as argument and return the sum of all of it as float.

sum_f = sum_f(3.14, 0.1, 0.2)

# `integer sum_ai([]integer...)`, receive an `array of integers` as argument and return the sum of all it's items as `integer`.
sum_ai = sum_ai([10, 30, 50])

# `float sum_af([]float...)`, receive an `array of floats` as argument and return the sum of all its items as `float`.
sum_af = sum_af([3.14, 0.1, 0.2])

# `[]any concat_a([]any...)`, receive N `arrays` as argument and return all arrays concatenated.

array_a = [3.14, 0.1, 0.2]
array_b = [10, 30, 50, 100]

concat_a = concat_a(
	$/array_a,
	[$/array_b[2]],
	[concat_a(["Hello"], ["World"])[0], "testing"]
)

# `string concat_s(string...)`, receive N `strings` as argument and return a single string concatenated.
user_name = "World"
concat_s = concat_s("Hello ", $/user_name, ". How are you?")

# `string join_as([]string, string?)`, receive an array of strings and a separator (which can be nil) then return a joined string.
# if separator is `nil`, the array will be joined without counting with the separator.
join_as_one = join_as(["Hello", $/user_name, "How are you?"], ", ")
join_as_two = join_as(["Hello", $/user_name, "How are you?"], nil)

# `integer len(string)`, receive a string as argument and return the length of it
join_as_one_len = len($/join_as_one)
join_as_two_len = len($/join_as_two)

# `[]string keys(object)`, receive an object as argument and return an array of strings with all its keys
address = { object = { city = "New York", country = "USA", "street name" = "Something" } }
keys = keys($/address["object"])

# `integer len(array)`, receive an array as argument and return the length of it
array = [[1, 2, 3, 4, 6, 34, 2, 9]]
array_len = len($/array[0])

# `integer iota()`, returns an integer which auto-increment every time it's called
iota_a = iota()
iota_b = iota()
iota_c = iota()
iota_d = iota()
