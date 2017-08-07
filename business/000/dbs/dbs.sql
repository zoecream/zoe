/* 流水号表 */
create table JournalNo
(
	TrnJour  integer  not null /* 交易流水 */
)
/* 流水表 */
create table Journal
(
	BsnCode     char(3)  not null,/* 业务代码 */
	CliLnkCode  char(3)  not null,/* 客户端渠道代码 */
	CliTrnCode  char(15) not null,/* 客户端交易代码 */
	TrnDate     char(8)  not null,/* 交易日期 */
	TrnTime     char(6)  not null,/* 交易时间 */
	TrnJour     char(31) not null,/* 交易流水 */
	CliRetStat  char(1)  not null,/* 客户端结果状态 */
	CliRetCode  char(8)  not null,/* 客户端结果代码 */
	CliRetInfo  char(63) not null,/* 客户端结果信息 */

	primary key (TrnJour)
)
