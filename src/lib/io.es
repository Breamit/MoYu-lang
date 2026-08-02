
@native('io.print')
fn print(str:string):void;

@native('io.println')
fn println(str:string):void;

@native('io.getStr')
fn getStr(str:string):void;

@native('io.getLine')
fn getLine(str:string):void;

@native('io.getFloat')
fn getFloat(str:string):void;

@native('io.getInt')
fn getInt(str:string):void;

export {
    print,println,getStr,getLine,getFloat,getInt
};
