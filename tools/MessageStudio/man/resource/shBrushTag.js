dp.sh.Brushes.Tag = function()
{
    this.regexList = [
 {regex: dp.sh.RegexLib.DoubleQuotedString, css: 'string'},
 {regex: new RegExp('[()[\\]{}]', 'g'), css: 'delim'},
 {regex: new RegExp(':\\w+', 'g'), css: 'keyword'}
 ];

    this.CssClass = 'dp-tag';

    this.Style =    '.dp-tag .delim {font-weight: bold;}' +
                    '.dp-tag .string {color: red;}';
}

dp.sh.Brushes.Tag.prototype = new dp.sh.Highlighter();
dp.sh.Brushes.Tag.Aliases = ['tag'];
