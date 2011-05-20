dp.sh.Brushes.Reg = function()
{
    this.regexList = [
 {regex: dp.sh.RegexLib.DoubleQuotedString, css: 'string'},
 {regex: new RegExp('[()[\\]{}]', 'g'), css: 'delim'},
 {regex: new RegExp(':\\w+', 'g'), css: 'keyword'},
 ];

    this.CssClass = 'dp-tag';

    this.Style =    '.dp-tag .delim {font-weight: bold;}' +
                    '.dp-tag .string {color: blue;}';
}

dp.sh.Brushes.Reg.prototype = new dp.sh.Highlighter();
dp.sh.Brushes.Reg.Aliases = ['reg'];
