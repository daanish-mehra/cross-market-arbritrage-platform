'use client'

interface ArbitrageOpportunity {
  event_id: string
  buy_market: number
  sell_market: number
  buy_price: number
  sell_price: number
  profit_percentage: number
  max_size: number
}

interface OpportunitiesListProps {
  opportunities: ArbitrageOpportunity[]
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

export default function OpportunitiesList({ opportunities }: OpportunitiesListProps) {
  return (
    <div className="card">
      <h2>Arbitrage Opportunities</h2>
      {opportunities.length === 0 ? (
        <p style={{ color: '#999' }}>No opportunities detected yet. Waiting for market data...</p>
      ) : (
        opportunities.map((opp, index) => (
          <div key={index} className={`opportunity ${opp.profit_percentage > 0.01 ? 'profitable' : ''}`}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.5rem' }}>
              <span style={{ fontWeight: 600 }}>{opp.event_id}</span>
              <span className="profit-badge">
                {opp.profit_percentage.toFixed(2)}% profit
              </span>
            </div>
            <div style={{ fontSize: '0.875rem', color: '#999', marginBottom: '0.25rem' }}>
              Buy: {marketNames[opp.buy_market] || `Market ${opp.buy_market}`} @ ${opp.buy_price.toFixed(4)}
            </div>
            <div style={{ fontSize: '0.875rem', color: '#999', marginBottom: '0.25rem' }}>
              Sell: {marketNames[opp.sell_market] || `Market ${opp.sell_market}`} @ ${opp.sell_price.toFixed(4)}
            </div>
            <div style={{ fontSize: '0.875rem', color: '#666' }}>
              Max size: ${opp.max_size.toFixed(2)}
            </div>
          </div>
        ))
      )}
    </div>
  )
}

