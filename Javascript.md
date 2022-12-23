



## Supported features

- Operations: all standard JS operations except:
   - `!=`, `==`. Use strict comparison `!==`, `===`
   - No computed member access `a[b]`
   - No exponentiation operation `a ** b`
- Typeof: `typeof('a') === 'string'`
- For loop: `for (...;...;...)  ...`
- Conditional: `if (...) ... else ...`
- Ternary operator `a ? b : c`
- Simple types: `let a, b, c = 12.3, d = 'a', e = null, f = true, g = false;`
- Functions: `let f = function(x, y) { return x + y; };`
- Objects: `let obj = {f: function(x) { return x * 2}}; obj.f(3);`
- Every statement must end with a semicolon `;`
- Strings are binary data chunks, not Unicode strings: `'Київ'.length === 8`

## Not supported features

- No `var`, no `const`. Use `let` (strict mode only)
- No `do`, `switch`, `while`. Use `for`
- No `=>` functions. Use `let f = function(...) {...};`
- No arrays, closures, prototypes, `this`, `new`, `delete`
- No standard library: no `Date`, `Regexp`, `Function`, `String`, `Number`


## Available functions

This editor lets you customise ESP32 firmware with JavaScript. All components are open source:

elk - a JS engine
mongoose - a networking library
Eps32JS - ESP32 firmware and this editor
C API imported to JS (Default):
- gpio.mode(pin, mode) - set pin mode
- gpio.write(pin, value) - set pin state
- gpio.read(pin) - read pin state
- timer.create(milli, func, null) - create timer
- timer.delete(timerID) - delete timer
- log(strval) - log a message

C API imported to JS (Proprietary):
- sensor.read(ref) // get last value of sensor mapped with ref id string
- sensor.read_rs485(unit_id,fc,address,len) // real time reads directly from bus
- sensor.read_rs485_all() // read all registered rs485 sensors
- date.now() // unix timestamp
- date.millis() // millis()
- mqtt.send() // send messages to mqtt broker

Note:
- log method send all data to mqtt broker with topic /js/log
- topic /js/debug will display all debug messages from runtime js code
